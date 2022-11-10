"""This module defines the cipd_install repository rule.

The cipd_install rule is a wrapper around http_archive, as we download the CIPD
package at the specified version over HTTPS.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cipd_install(name, build_file_content, cipd_package, sha256, tag):
    """Download and extract the zippd archive from CIPD

    Args:
        name: The name of the Bazel "repository" created. For example, if name is "alpha_beta",
              the full Bazel label will start with @alpha_beta//
        build_file_content: CIPD packages do not come with BUILD.bazel files, so we must supply
                            one. This should generally be export_files or filegroup.
        cipd_package: The full name of the CIPD package. This is a "path" from the root of CIPD.
                      For ease of use, this should be a publicly visible package.
        sha256: The sha256 hash of the zip archive downloaded from CIPD. This should match the
                official CIPD website.
        tag: Represents the version of the CIPD package to download.
             For example, git_package:...
    """
    cipd_url = "https://chrome-infra-packages.appspot.com/dl/"
    cipd_url += cipd_package
    cipd_url += "/+/"
    cipd_url += tag

    mirror_url = "https://storage.googleapis.com/skia-world-readable/bazel/"
    mirror_url += sha256
    mirror_url += ".zip"
    http_archive(
        name = name,
        build_file_content = build_file_content,
        sha256 = sha256,
        urls = [
            cipd_url,
            mirror_url,
        ],
        type = "zip",
    )
