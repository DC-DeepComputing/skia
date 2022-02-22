const path = require('path');
const fs = require('fs')
// This should be a file created by gold_test_env.go which contains the port number
// on which it is listening. For whatever reason, karma was not happy serving the
// port file directly, but reading it in and then adding it as a proxy seems to
// work fine.
const testOnEnvPortPath = path.join(process.env['ENV_DIR'], 'port');
const port = fs.readFileSync(testOnEnvPortPath, 'utf8').toString();
console.log('test_on_env PORT:', port);

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
      // The tests will make calls to /gold_rpc/whatever and they will be redirected
      // to the correct location.
      '/gold_rpc/': `http://localhost:${port}/`,
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
  };

  // Bazel will inject some code here to set up things like which browser to target.
  BAZEL_APPLY_SETTINGS(cfg);

  config.set(cfg);
};
