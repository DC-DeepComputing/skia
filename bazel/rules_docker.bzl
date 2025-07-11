"""
rules_docker is deprecated and not in the Bazel registry, so
we need to manually bring it and its deps in.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("//bazel:gcs_mirror.bzl", "gcs_mirror_url")

def _rules_docker_impl(_ctx):
    # https://bazel.build/rules/lib/builtins/module_ctx

    http_archive(
        name = "io_bazel_rules_docker",
        sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
        urls = gcs_mirror_url(
            sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
            url = "https://github.com/bazelbuild/rules_docker/releases/download/v0.25.0/rules_docker-v0.25.0.tar.gz",
        ),
    )

    # These are the pinned version that io_bazel_rules_docker depends
    # on and are distinct from the versions in MODULE.bazel
    # https://github.com/bazelbuild/rules_docker/blob/7215260897250c2774fcec6d1e7c39c83886d27d/repositories/repositories.bzl
    # http_archive(
    #     name = "io_bazel_rules_go",
    #     sha256 = "08c3cd71857d58af3cda759112437d9e63339ac9c6e0042add43f4d94caf632d",
    #     urls = [
    #         "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/v0.24.2/rules_go-v0.24.2.tar.gz",
    #         "https://github.com/bazelbuild/rules_go/releases/download/v0.24.2/rules_go-v0.24.2.tar.gz",
    #     ],
    # )
    CONTAINERREGISTRY_RELEASE = "v0.0.38"
    RULES_DOCKER_GO_BINARY_RELEASE = "aad94363e63d31d574cf701df484b3e8b868a96a"

    http_file(
        name = "go_puller_linux_amd64",
        executable = True,
        sha256 = "08b8963cce9234f57055bafc7cadd1624cdce3c5990048cea1df453d7d288bc6",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/puller-linux-amd64")],
    )

    http_file(
        name = "go_puller_linux_arm64",
        executable = True,
        sha256 = "912ee7c469b3e4bf15ba5d1f0ee500e7ec6724518862703fa8b09e4d58ce3ee6",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/puller-linux-arm64")],
    )

    http_file(
        name = "go_puller_linux_s390x",
        executable = True,
        sha256 = "a5527b7b3b4a266e4680a4ad8939429665d4173f26b35d5d317385134369e438",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/puller-linux-s390x")],
    )

    http_file(
        name = "go_puller_darwin",
        executable = True,
        sha256 = "4855c4f5927f8fb0f885510ab3e2a166d5fa7cde765fbe9aec97dc6b2761bb22",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/puller-darwin-amd64")],
    )

    http_file(
        name = "loader_linux_amd64",
        executable = True,
        sha256 = "5e5ada66beff07f9188bdc1f99c3fa37c407fc0048cd78b9c2047e9c5516f20b",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/loader-linux-amd64")],
    )

    http_file(
        name = "loader_linux_arm64",
        executable = True,
        sha256 = "a80966d17b25dbc9313e9fc1cae74ded5916fa64dba0d33438c8adad338b44d3",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/loader-linux-arm64")],
    )

    http_file(
        name = "loader_linux_s390x",
        executable = True,
        sha256 = "0c0ebc3e0a502542547a38b51f4686a049897eeb4cbc0e2f07fc25276c57866f",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/loader-linux-s390x")],
    )

    http_file(
        name = "loader_darwin",
        executable = True,
        sha256 = "8c9986b2b506febbff737090d9ec485cec1376c52789747573521a85194341c1",
        urls = [("https://storage.googleapis.com/rules_docker/" + RULES_DOCKER_GO_BINARY_RELEASE + "/loader-darwin-amd64")],
    )

    http_archive(
        name = "containerregistry",
        sha256 = "a0c01fcc11db848212f8b11d89df168361f99a31eb7373ff60ce50c5d05cd74b",
        strip_prefix = "containerregistry-" + CONTAINERREGISTRY_RELEASE[1:],
        urls = [("https://github.com/google/containerregistry/archive/" +
                 CONTAINERREGISTRY_RELEASE + ".tar.gz")],
    )

rules_docker = module_extension(
    implementation = _rules_docker_impl,
)
