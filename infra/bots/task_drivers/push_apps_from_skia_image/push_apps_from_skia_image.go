// Copyright 2021 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This executable builds the Docker images based off the Skia executables in the
// gcr.io/skia-public/skia-release image. It then issues a PubSub notification to have those apps
// tagged and deployed by docker_pushes_watcher.
// See //docker_pushes_watcher/README.md in the infra repo for more.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"os"
	"path"
	"path/filepath"

	"cloud.google.com/go/pubsub"
	"google.golang.org/api/option"

	"go.skia.org/infra/go/auth"
	docker_pubsub "go.skia.org/infra/go/docker/build/pubsub"
	sk_exec "go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/bazel"
	"go.skia.org/infra/task_driver/go/lib/checkout"
	"go.skia.org/infra/task_driver/go/lib/docker"
	"go.skia.org/infra/task_driver/go/lib/golang"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var (
	// Required properties for this task.
	projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
	taskId    = flag.String("task_id", "", "ID of this task.")
	taskName  = flag.String("task_name", "", "Name of the task.")
	workdir   = flag.String("workdir", ".", "Working directory")

	checkoutFlags = checkout.SetupFlags(nil)

	// Optional flags.
	local  = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
	output = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
)

const (
	fiddlerImageName = "fiddler"
	apiImageName     = "api"

	infraRepo = "https://skia.googlesource.com/buildbot.git"
	skiaRepo  = "https://skia.googlesource.com/skia.git"
)

var (
	infraCommonEnv = []string{
		"SKIP_BUILD=1",
		"ROOT=/OUT",
	}
	infraCommonBuildArgs = map[string]string{
		"SKIA_IMAGE_NAME": "skia-release",
	}
)

func buildPushFiddlerImage(ctx context.Context, tag, repo, configDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	image := fmt.Sprintf("gcr.io/skia-public/%s", fiddlerImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/fiddlek && ./build_fiddler_release"}
	volumes := []string{fmt.Sprintf("%s:/OUT", tempDir)}
	err = docker.BuildPushImageFromInfraImage(ctx, "Fiddler", image, tag, repo, configDir, tempDir, "prod", topic, cmd, volumes, infraCommonEnv, infraCommonBuildArgs)
	if err != nil {
		return err
	}
	return cleanupTempFiles(ctx, configDir, volumes)
}

// Not needed?
func cleanupTempFiles(ctx context.Context, configDir string, volumes []string) error {
	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	const infraImageWithTag = "gcr.io/skia-public/infra:prod"
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	return docker.Run(ctx, infraImageWithTag, configDir, cleanupCmd, volumes, nil)
}

func buildPushApiImage(ctx context.Context, tag, repo, configDir, checkoutDir string, topic *pubsub.Topic) error {
	tempDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		return err
	}
	// Change perms of the directory for doxygen to be able to write to it.
	if err := os.Chmod(tempDir, 0777); err != nil {
		return err
	}
	// Run Doxygen pointing to the location of the checkout and the out dir.
	volumes := []string{
		fmt.Sprintf("%s:/OUT", tempDir),
		fmt.Sprintf("%s:/CHECKOUT", checkoutDir),
	}
	env := []string{
		"OUTPUT_DIRECTORY=/OUT",
	}
	doxygenCmd := []string{"/bin/sh", "-c", "cd /CHECKOUT/tools/doxygen && doxygen ProdDoxyfile"}
	// DO THIS BUT USE "dkr" instead!
	if err := docker.Run(ctx, "gcr.io/skia-public/doxygen:testing-slim", configDir, doxygenCmd, volumes, env); err != nil {
		return err
	}

	image := fmt.Sprintf("gcr.io/skia-public/%s", apiImageName)
	cmd := []string{"/bin/sh", "-c", "cd /home/skia/golib/src/go.skia.org/infra/api && make release_ci"}
	infraEnv := util.CopyStringSlice(infraCommonEnv)
	infraEnv = append(infraEnv, "DOXYGEN_HTML=/OUT/html")
	infraVolumes := []string{fmt.Sprintf("%s:/OUT", tempDir)}
	err = docker.BuildPushImageFromInfraImage(ctx, "Api", image, tag, repo, configDir, tempDir, "prod", topic, cmd, infraVolumes, infraEnv, infraCommonBuildArgs)
	if err != nil {
		return err
	}
	return cleanupTempFiles(ctx, configDir, volumes)
}

func main() {
	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	rs, err := checkout.GetRepoState(checkoutFlags)
	if err != nil {
		td.Fatal(ctx, err)
	}

	wd, err := os_steps.Abs(ctx, *workdir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Check out the code.
	co, err := checkout.EnsureGitCheckout(ctx, path.Join(wd, "repo"), rs)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Setup go.
	ctx = golang.WithEnv(ctx, wd)

	// Create token source with scope for cloud registry (storage) and pubsub.
	ts, err := auth_steps.Init(ctx, *local, auth.ScopeUserinfoEmail, auth.ScopeFullControl, pubsub.ScopePubSub)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Create pubsub client.
	client, err := pubsub.NewClient(ctx, docker_pubsub.TOPIC_PROJECT_ID, option.WithTokenSource(ts))
	if err != nil {
		td.Fatal(ctx, err)
	}
	topic := client.Topic(docker_pubsub.TOPIC)

	// Figure out which tag to use for the skia_release image.
	tag := rs.Revision
	if rs.Issue != "" && rs.Patchset != "" {
		tag = fmt.Sprintf("%s_%s", rs.Issue, rs.Patchset)
	}

	// Instantiate docker.
	dkr, err := docker.New(ctx, ts)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// Run skia-release image and extract products out of it.
	skiaProductsDir, err := os_steps.TempDir(ctx, "", "")
	if err != nil {
		td.Fatal(ctx, err)
	}
	volumes := []string{
		fmt.Sprintf("%s:/OUT", skiaProductsDir),
	}
	skiaCopyCmd := []string{"/bin/sh", "-c", "cp -r /tmp/* /OUT"}
	releaseImg := fmt.Sprintf("gcr.io/skia-public/skia-release:%s", tag)
	if err := dkr.Run(ctx, releaseImg, skiaCopyCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}

	// Ensure that the bazel cache is setup.
	opts := bazel.BazelOptions{
		CachePath: "/mnt/pd0/bazel_cache",
	}
	if err := bazel.EnsureBazelRCFile(ctx, opts); err != nil {
		td.Fatal(ctx, err)
	}

	// // Add the tag to infraCommonBuildArgs.
	// infraCommonBuildArgs["SKIA_IMAGE_TAG"] = tag

	// // Create a temporary config dir for Docker.
	// configDir, err := ioutil.TempDir("", "")
	// if err != nil {
	// 	td.Fatal(ctx, err)
	// }
	// defer util.RemoveAll(configDir)

	// // Login to docker (required to push to docker).
	// token, err := ts.Token()
	// if err != nil {
	// 	td.Fatal(ctx, err)
	// }
	// if err := docker.Login(ctx, token.AccessToken, "gcr.io/skia-public/", configDir); err != nil {
	// 	td.Fatal(ctx, err)
	// }

	// Build and push all apps of interest below.
	if err := buildPush(ctx, fiddlerImageName, skiaProductsDir, co.Dir(), tag, topic); err != nil {
		td.Fatal(ctx, err)
	}
	// CANNOT USE ABOCVE RIGHT NOW BECUAE THIS IS DOING SOME DOXYGEN STUFF!
	if err := buildPushApiImage(ctx, tag, rs.Repo, co.Dir(), tag, topic); err != nil {
		td.Fatal(ctx, err)
	}

	// NEW
	// Remove all temporary files from the host machine. Swarming gets upset if there are root-owned
	// files it cannot clean up.
	cleanupCmd := []string{"/bin/sh", "-c", "rm -rf /OUT/*"}
	if err := dkr.Run(ctx, releaseImg, cleanupCmd, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}
}

// PUT IN A UTIL!
// buildPush pushes the app in the given skia-infra folder using `make bazel_release_ci`.
// By convention, all of these apps should have the k8s deployment name be the same as the app
// name and be built using the same Make target.
func buildPush(ctx context.Context, appName, productsDir, checkoutDir, skiaRevision string, topic *pubsub.Topic) error {
	err := td.Do(ctx, td.Props("Build "+appName+" image").Infra(), func(ctx context.Context) error {
		runCmd := &sk_exec.Command{
			Name:       "make",
			Args:       []string{"bazel_release_ci"},
			InheritEnv: true,
			Env: []string{
				"COPY_FROM_DIR=" + productsDir,
				"STABLE_DOCKER_TAG=" + skiaRevision,
			},
			Dir:       filepath.Join(checkoutDir, appName),
			LogStdout: true,
			LogStderr: true,
		}
		_, err := sk_exec.RunCommand(ctx, runCmd)
		if err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	return publishToTopic(ctx, "gcr.io/skia-public/"+appName, skiaRevision, topic)
}

// Update to use new docker.PublicToTopic
func publishToTopic(ctx context.Context, image, tag string, topic *pubsub.Topic) error {
	return td.Do(ctx, td.Props(fmt.Sprintf("Publish pubsub msg to %s", docker_pubsub.TOPIC)).Infra(), func(ctx context.Context) error {
		b, err := json.Marshal(&docker_pubsub.BuildInfo{
			ImageName: image,
			Tag:       tag,
			Repo:      skiaRepo,
		})
		if err != nil {
			return err
		}
		msg := &pubsub.Message{
			Data: b,
		}
		res := topic.Publish(ctx, msg)
		// Synchronously wait to make sure the publishing actually happens.
		if _, err := res.Get(ctx); err != nil {
			return err
		}
		return nil
	})
}
