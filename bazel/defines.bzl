"""
This file contains customizable C++ defines.

The primary use case for this file is to have defines we can control when rolling into G3.
The defines for G3 may pull from public.bzl, allowing us to atomically change defines when
rolling and not have to always do manual rolls to update them.

"""

EXTRA_DEFINES = []
