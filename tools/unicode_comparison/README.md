Unicode comparison utilities
============================

This directory contains the following

cpp/*
-----
This folder contains the bridge code between SkUnicode and golang

go/*
----
This folder contains the set on golang utilities that allow to download, preprocess data and then generate the comparison table itself

download_wiki.go
----------------
make download_wiki
[Release]/download_wiki
You may need to update go-wiki package: go get -u github.com/trietmn/go-wiki
List of wiki languages: https://meta.wikimedia.org/wiki/List_of_Wikipedias

extract_info.go
---------------
make extract_info
[Release]/extract_info --impl icu
[Release]/extract_info --impl icu4x
[Release]/extract_info --impl libgrapheme

In datasets folderyou should pick one of the subfolders of outputs (probably, icu), move it up
under datasets itself and rename into "validation". That would be your validation set.

generate_table.go
-----------------
make generate_table
[Release]/generate_table

It will put under datasets index.html that contains the comparison results.

