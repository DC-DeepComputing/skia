// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"fmt"
	"os"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/gold-client/go/auth"
	"go.skia.org/infra/gold-client/go/goldclient"
	"go.skia.org/infra/golden/go/jsonio"
	"go.skia.org/infra/golden/go/types"
)

type goldClient struct {
	ctx context.Context
	gc  *goldclient.CloudClient
}

type goldClientProps struct {
	DryRun  bool
	WorkDir string
}

func newGoldClient(ctx context.Context, props goldClientProps) (*goldClient, error) {
	props.DryRun = true

	if err := os.MkdirAll(props.WorkDir, 0o755); err != nil {
		return nil, err
	}

	var err error
	if props.DryRun {
		err = auth.InitNoAuth(props.WorkDir)
	} else {
		err = auth.InitLUCIAuth(props.WorkDir)
	}
	if err != nil {
		return nil, skerr.Wrapf(err, "gold auth")
	}

	a, err := auth.LoadAuthOpt(props.WorkDir)
	if err != nil {
		return nil, err
	}

	a.SetDryRun(props.DryRun)
	up, err := a.GetGCSUploader(ctx)
	if err != nil {
		return nil, err
	}

	hc, err := a.GetHTTPClient()
	if err != nil {
		return nil, err
	}

	dl, err := a.GetImageDownloader()
	if err != nil {
		return nil, err
	}

	gctx := goldclient.WithContext(ctx, up, hc, dl)
	gc, err := goldclient.NewCloudClient(goldclient.GoldClientConfig{
		WorkDir:         props.WorkDir,
		InstanceID:      "skia",
		OverrideGoldURL: "https://gold.skia.org",
		OverrideBucket:  "skia-infra-gm",
		UploadOnly:      true,
	})
	if err != nil {
		return nil, err
	}

	return &goldClient{
		ctx: gctx,
		gc:  gc,
	}, nil
}

func (g *goldClient) Init(gr *jsonio.GoldResults, digestOnly []jsonio.Result) error {
	// This initializes the client with:
	// - shared keys
	// - commit/tryjob info
	// - results that reference existing digests (i.e. results do not have image
	//   data attached), whether preexisting on gold or by a test image that will
	//   be uploaded before the next call to Finalize.
	shared := *gr
	shared.Results = digestOnly
	if err := g.gc.SetSharedConfig(g.ctx, shared, false); err != nil {
		return fmt.Errorf("gold client SetSharedConfig: %w", err)
	}

	return nil
}

func (g *goldClient) UploadImage(name types.TestName, pngPath string, digest types.Digest, additionalKeys, optionalKeys map[string]string) error {
	_, err := g.gc.Test(g.ctx, name, pngPath, digest, additionalKeys, optionalKeys)
	return err
}

func (g *goldClient) Finalize() error {
	return g.gc.Finalize(g.ctx)
}
