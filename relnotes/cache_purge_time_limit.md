* Allow clients to pass in a maximum time duration Skia should spend when purging purgeable
  resources from caches. Skia will purge available purgeable resources until all purgeable resources
  are removed or until the purge time has been surpassed, at which point we exit early.
* The stop time pointer parameter has a default value of nullptr, signifying no time limit for
  purging. This means this change should not break current client API calls nor impact existing
  functionality.

