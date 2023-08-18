# /usr/bin/env python3

import requests
import json
import os
from multiprocessing.pool import Pool


SAMPLE_FONTS = [
    "Noto Sans",
    "Noto Sans JP",
    "Noto Sans Malayalam",
    "Noto Sans Devanagari",
    "Noto Sans Thai",
]

LIST_URL_TEMPLATE = "https://fonts.google.com/download/list?family={}"
SAMPLE_URL_TEMPLATE = "https://fonts.google.com/sampletext?family={}"
OUTPUT_FOLDER = "fontations_test_fonts/"
JSON_DUMP_FILE = "fontations_test_data.json"


def regular_version_source_location(font_name):
    fetch_result = requests.get(
        LIST_URL_TEMPLATE.format(requests.utils.quote(font_name))
    )
    # TODO: Why does the curl request result in 4 garbage brackets at the beginning?
    json_result = json.loads(fetch_result.text[4:])
    files = json_result["manifest"]["fileRefs"]
    regular_font_filerefs = list(filter(lambda x: "Regular" in x["filename"], files))
    return min(regular_font_filerefs, key=lambda entry: entry["filename"])


def samples_for_font(font_name):
    fetch_result = requests.get(
        SAMPLE_URL_TEMPLATE.format(requests.utils.quote(font_name))
    )
    # TODO: Why does the curl request result in 4 garbage brackets at the beginning?
    json_result = json.loads(fetch_result.text[4:])
    return json_result


def download_to_fonts_dir(url):
    r = requests.get(url, allow_redirects=True)
    filename = url.split("/")[-1]
    with open(OUTPUT_FOLDER + filename, "wb") as dl_file:
        dl_file.write(r.content)


def fetch_font_and_samples(font_name):
    regular_source = regular_version_source_location(font_name)
    text_samples = samples_for_font(font_name)
    download_to_fonts_dir(regular_source["url"])
    return {
        "fontName": font_name,
        "fontRelPath": OUTPUT_FOLDER + os.path.basename(regular_source["url"]),
        "samples": text_samples,
    }


if __name__ == "__main__":
    test_data_dict = []
    with Pool(4) as pool:
        for per_font_result in pool.map(fetch_font_and_samples, SAMPLE_FONTS):
            test_data_dict.append(per_font_result)
    with open(JSON_DUMP_FILE, "w") as json_info_file:
        json_info_file.write(json.dumps(test_data_dict, indent=True))
