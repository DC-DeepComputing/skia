LINUX_VERSION=v1

push_linux_container:
	docker build -t gcr.io/skia-public/rbe_linux:${LINUX_VERSION} ./gce_linux_container/
	docker push gcr.io/skia-public/rbe_linux:${LINUX_VERSION}


generate_linux_config:
	rbe_configs_gen \
		--bazel_version=5.0.0 \
		--toolchain_container=gcr.io/skia-public/rbe_linux@sha256:e345c670863976a6bc9cfd4676fb2049b0ea7035c6c87caf2ee8b5e2629e7f92 \
		--output_src_root=../.. \
		--output_config_path=bazel/rbe \
		--exec_os=linux \
		--target_os=linux