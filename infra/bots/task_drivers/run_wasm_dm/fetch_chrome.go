// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"path"
	r "runtime"
	"time"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"
)

type chromeDlSpec struct {
	platform string
	exePath  string
}

type chromeVersions struct {
	Timestamp time.Time       `json:"timestamp"`
	Versions  []chromeVersion `json:"versions"`
}

type chromeVersion struct {
	Version   string           `json:"version"`
	Revision  string           `json:"revision"`
	Downloads versionDownloads `json:"downloads"`
}

type versionDownloads struct {
	Chrome []versionDownload `json:"chrome"`
}

type versionDownload struct {
	Platform string `json:"platform"`
	URL      string `json:"url"`
}

func installChrome(ctx context.Context, workPath, installDir string) (string, error) {
	ctx = td.StartStep(ctx, td.Props("install chrome").Infra())
	defer td.EndStep(ctx)

	latestVersion, err := fetchChromeLatestVersionNum()
	if err != nil {
		return "", td.FailStep(ctx, err)
	}
	sklog.Infof("latest chrome version is %s", latestVersion)

	versions, err := fetchChromeVersions()
	if err != nil {
		return "", td.FailStep(ctx, err)
	}

	spec, err := determineChromeDownloadSpec()
	if err != nil {
		return "", td.FailStep(ctx, err)
	}

	zipUrl := ""
	for _, v := range versions.Versions {
		if v.Version == latestVersion {
			for _, p := range v.Downloads.Chrome {
				if p.Platform == spec.platform {
					zipUrl = p.URL
					break
				}
			}
		}
		if len(zipUrl) > 0 {
			break
		}
	}
	if len(zipUrl) == 0 {
		return "", td.FailStep(ctx, fmt.Errorf("failed to find zip url for chrome %s platform %s", latestVersion, spec.platform))
	}

	sklog.Infof("fetching chrome %s...", zipUrl)
	zipfile := path.Join(workPath, "chrome.zip")
	if err := downloadFile(zipUrl, zipfile); err != nil {
		return "", td.FailStep(ctx, err)
	}

	if err := extractZip(ctx, zipfile, installDir); err != nil {
		return "", td.FailStep(ctx, err)
	}

	chromeBin := path.Join(installDir, spec.exePath)
	return chromeBin, nil
}

func fetchChromeLatestVersionNum() (string, error) {
	url := "https://googlechromelabs.github.io/chrome-for-testing/LATEST_RELEASE_STABLE"
	resp, err := http.Get(url)
	if err != nil {
		return "", fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", fmt.Errorf("failed to read body: %w", err)
	}
	return string(body), nil
}

func fetchChromeVersions() (*chromeVersions, error) {
	url := "https://googlechromelabs.github.io/chrome-for-testing/known-good-versions-with-downloads.json"
	resp, err := http.Get(url)
	if err != nil {
		return nil, fmt.Errorf("failed to fetch %s: %w", url, err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to fetch %s: %s", url, resp.Status)
	}

	var versions chromeVersions
	if err := json.NewDecoder(resp.Body).Decode(&versions); err != nil {
		return nil, fmt.Errorf("failed to decode %s: %w", url, err)
	}

	return &versions, nil
}

func determineChromeDownloadSpec() (chromeDlSpec, error) {
	platforms := map[string]map[string]string{
		"darwin":  {"amd64": "mac-x64", "arm64": "mac-arm64"},
		"linux":   {"amd64": "linux64"},
		"windows": {"386": "win32", "amd64": "win64"},
	}

	exes := map[string]string{
		"darwin":  "Google Chrome for Testing.app/Contents/MacOS/Google Chrome for Testing",
		"linux":   "chrome",
		"windows": "chrome.exe",
	}

	if platform, ok := platforms[r.GOOS][r.GOARCH]; ok {
		exePath := path.Join(fmt.Sprintf("chrome-%s", platform), exes[r.GOOS])
		return chromeDlSpec{
			platform: platform,
			exePath:  exePath,
		}, nil
	}
	return chromeDlSpec{}, fmt.Errorf("no known chrome platform for %s/%s", r.GOOS, r.GOARCH)
}

func downloadFile(url, destPath string) error {
	resp, err := http.Get(url)
	if err != nil {
		return fmt.Errorf("failed to fetch: %w", err)
	}
	defer resp.Body.Close()

	f, err := os.Create(destPath)
	if err != nil {
		return fmt.Errorf("failed to create file: %w", err)
	}
	defer f.Close()

	if _, err := io.Copy(f, resp.Body); err != nil {
		return fmt.Errorf("failed to write: %w", err)
	}

	return nil
}

func extractZip(ctx context.Context, zipPath, targetDir string) error {
	runCmd := &exec.Command{
		Name:      "unzip",
		Args:      []string{"-oq", zipPath, "-d", targetDir},
		LogStderr: true,
	}
	_, err := exec.RunCommand(ctx, runCmd)
	if err != nil {
		return err
	}
	return nil
}
