LINUX_VERSION="v1"

push_linux_container:
	docker build -t gcr.io/skia-public/rbe_linux:${LINUX_VERSION} ./gce_linux_container/
	docker push gcr.io/skia-public/rbe_linux:${LINUX_VERSION}


generate_linux_config:
	rbe_configs_gen \
		--bazel_version=5.0.0 \
		--toolchain_container=gcr.io/skia-public/rbe_linux@sha256:f46fd88b7d9393a6fcc3012485750c22842c4d1536c2d18647ff67e4907dc370 \
		--output_src_root=../.. \
		--output_config_path=bazel/rbe \
		--exec_os=linux \
		--target_os=linux