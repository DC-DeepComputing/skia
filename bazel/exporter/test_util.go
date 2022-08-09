// Copyright 2022 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exporter

import (
	"go.skia.org/infra/go/skerr"
	"go.skia.org/skia/bazel/exporter/build_proto/analysis_v2"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"
)

// A test helper function to convert a textproto protobuf into a binary protobuf.
func textProtoToProtobuf(textProto string) ([]byte, error) {
	qr := analysis_v2.CqueryResult{}
	err := prototext.Unmarshal([]byte(textProto), &qr)
	if err != nil {
		return nil, skerr.Wrapf(err, "unable to unmarshal textproto")
	}

	return proto.Marshal(&qr)
}
