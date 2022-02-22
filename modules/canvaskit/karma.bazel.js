module.exports = function(config) {
  // Set the default values to be what are needed when testing the
  // WebAssembly build locally.
  let cfg = {
    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],

    // list of files / patterns to load in the browser
    files: [],

    proxies: {},

    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress'],
    colors: true,
    logLevel: config.LOG_INFO,

    browserDisconnectTimeout: 20000,
    browserNoActivityTimeout: 20000,

    // Concurrency level
    // how many browser should be started simultaneous
    concurrency: Infinity,

    basePath: "BAZEL_BASE_PATH",
  };

  // Bazel will inject some code here to set up things like which browser to target.
  BAZEL_APPLY_SETTINGS(cfg)

  config.set(cfg);
};
