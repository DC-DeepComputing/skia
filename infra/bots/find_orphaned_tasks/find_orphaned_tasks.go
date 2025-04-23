/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package main

import (
	"context"
	"fmt"
	"net/url"
	"sort"
	"strings"

	apipb "go.chromium.org/luci/swarming/proto/api_v2"
	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/httputils"
	"go.skia.org/infra/go/sklog"
	swarmingv2 "go.skia.org/infra/go/swarming/v2"
	"go.skia.org/infra/task_scheduler/go/specs"
	"golang.org/x/oauth2/google"
)

const (
	server = "chromium-swarm.appspot.com"
)

func main() {
	ctx := context.Background()
	ts, err := google.DefaultTokenSource(ctx, auth.ScopeUserinfoEmail)
	if err != nil {
		sklog.Fatal(err)
	}
	httpClient := httputils.DefaultClientConfig().WithTokenSource(ts).Client()

	c := swarmingv2.NewDefaultClient(httpClient, server)

	cfg, err := specs.ReadTasksCfg(".")
	if err != nil {
		sklog.Fatal(err)
	}
	dimSets := map[string][]string{}
	swarmingLinks := map[string]string{}
	for taskName, taskCfg := range cfg.Tasks {
		sort.Strings(taskCfg.Dimensions)
		key := strings.Join(taskCfg.Dimensions, ";")
		dimSets[key] = append(dimSets[key], taskName)
		swarmingLinks[key] = makeSwarmingLink(taskCfg.Dimensions)
	}

	dimSetsWithNoBots := []string{}
	dimSetsWithDeadOrQuarantinedBots := []string{}
	for dimSet := range dimSets {
		bots, err := c.BotsClient.ListBots(ctx, &apipb.BotsRequest{
			Dimensions: makeDimensions(dimSet),
		})
		if err != nil {
			sklog.Fatal(err)
		}
		if len(bots.Items) == 0 {
			dimSetsWithNoBots = append(dimSetsWithNoBots, dimSet)
		}
		noBotsAvailable := true
		for _, bot := range bots.Items {
			if !bot.IsDead && !bot.Quarantined {
				noBotsAvailable = false
			}
		}
		if noBotsAvailable {
			dimSetsWithDeadOrQuarantinedBots = append(dimSetsWithDeadOrQuarantinedBots, dimSet)
		}
	}

	if len(dimSetsWithNoBots) > 0 {
		sort.Strings(dimSetsWithNoBots)
		fmt.Println("These tasks have no defined bots which can run them:")
		for _, key := range dimSetsWithNoBots {
			fmt.Println("  ", swarmingLinks[key])
			for _, task := range dimSets[key] {
				fmt.Println("    ", task)
			}
		}
	}

	if len(dimSetsWithDeadOrQuarantinedBots) > 0 {
		sort.Strings(dimSetsWithDeadOrQuarantinedBots)
		fmt.Println("These tasks have only dead or quarantined bots:")
		for _, key := range dimSetsWithDeadOrQuarantinedBots {
			fmt.Println("  ", swarmingLinks[key])
			for _, task := range dimSets[key] {
				fmt.Println("    ", task)
			}
		}
	}
}

func makeSwarmingLink(dims []string) string {
	terms := make([]string, 0, len(dims))
	for _, dim := range dims {
		terms = append(terms, fmt.Sprintf("f=%s", url.QueryEscape(dim)))
	}
	return fmt.Sprintf("https://chromium-swarm.appspot.com/botlist?%s", strings.Join(terms, "&"))
}

func makeDimensions(dims string) []*apipb.StringPair {
	split := strings.Split(dims, ";")
	rv := make([]*apipb.StringPair, 0, len(dims))
	for _, dim := range split {
		split := strings.SplitN(dim, ":", 2)
		rv = append(rv, &apipb.StringPair{
			Key:   split[0],
			Value: split[1],
		})
	}
	return rv
}
