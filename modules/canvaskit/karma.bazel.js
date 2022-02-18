const path = require('path');
// This should be a file created by gold_test_env.go which contains the port number
// on which it is listening.
const testOnEnvPortPath = path.join(process.env['ENV_DIR'], 'port');
console.log(process.env);

module.exports = function(config) {
  // Set the default values to be what are needed when testing the
  // WebAssembly build locally.
  let cfg = {
    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],

    // list of files / patterns to load in the browser
    files: [],

    proxies: {
      '/test_on_env_port': '/absolute'+testOnEnvPortPath,
    },

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

  console.log(cfg.proxies);

  config.set(cfg);
};
