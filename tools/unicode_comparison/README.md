Unicode comparison utilities
============================

This directory contains the following

cpp/*
-----
This folder contains the bridge code between SkUnicode and golang

go/*
----
This folder contains the set on golang utilities that allow to download, preprocess data and then generate the comparison table itself

<u>Make sure you have Skia release version built before building/running go utils!</u>

download_wiki.go
----------------
make download_wiki
../../../out/Release/download_wiki
You may need to update go-wiki package: go get -u github.com/trietmn/go-wiki
List of wiki languages: https://meta.wikimedia.org/wiki/List_of_Wikipedias

extract_info.go
---------------
make extract_info
../../../out/Release/extract_info --impl icu
../../../out/Release/extract_info --impl icu4x
../../../out/Release/extract_info --impl libgrapheme

Since currently there is no way to obtain a good validation set we use icu data as one.
So, copy ~/datasets/output/icu folder as ~/datasets/validation before the next step.
You may also delete ~/datasets/output/icu to avoid trivial comparison.

generate_table.go
-----------------
make generate_table
../../../out/Release/generate_table


