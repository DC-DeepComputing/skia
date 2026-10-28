---
name: skia/platform_tools/document-android-build-tool-version-compatibility
description: When configuring Gradle wrapper or Android Gradle Plugin versions in Android app buildscripts, document cross-tool version compatibility, Java requirements, and AGP documentation links
rubric: readability
severity: should_fix
---

**Rule.** When declaring or updating Gradle wrapper distributions
(distributionUrl) or Android Gradle Plugin dependencies
(com.android.tools.build:gradle) in platform_tools/android/apps/, document
coupled cross-file version compatibility, Java runtime requirements, and clarify
that the buildscript classpath version specifies AGP rather than Gradle itself,
linking to official Gradle and Android compatibility guides.

**Why.** Developers frequently confuse the Android Gradle Plugin (AGP) version
with the Gradle runtime version configured in gradle-wrapper.properties. Coupled
build tool versions across separate configuration files and host environment
dependencies (such as the installed Java runtime) lead to obscure build failures
if upgraded independently without consulting the required version compatibility
matrix.

## Examples

Bad:

```cpp
// In platform_tools/android/apps/gradle/wrapper/gradle-wrapper.properties:
distributionUrl=https\://services.gradle.org/distributions/gradle-8.14-all.zip

// In platform_tools/android/apps/build.gradle:
dependencies {
    classpath 'com.android.tools.build:gradle:8.9.0'
}
```

Good:

```cpp
// In platform_tools/android/apps/gradle/wrapper/gradle-wrapper.properties:
# The gradle version defined here must be compatible with the Android gradle plugin version defined
# in platform_tools/android/apps/build.gradle. Additionally, you must use a version of Java that is
# compatible per https://docs.gradle.org/current/userguide/compatibility.html.
distributionUrl=https\://services.gradle.org/distributions/gradle-8.14-all.zip

// In platform_tools/android/apps/build.gradle:
dependencies {
    // Note that this is a version number for the Android Gradle plugin described in
    // https://developer.android.com/build/releases/gradle-plugin, not Gradle itself.
    classpath 'com.android.tools.build:gradle:8.9.0'
}
```
