# /usr/bin/env python3

import requests
import json


SAMPLE_FONTS = [
    "Noto Sans",
    "Noto Sans JP",
    "Noto Sans Malayalam",
    "Noto Sans Devanagari",
    "Noto Sans Thai",
]

LIST_URL_TEMPLATE = "https://fonts.google.com/download/list?family="


def download_regular_version(font_name):
    fetch_result = requests.get(LIST_URL_TEMPLATE + requests.utils.quote(font_name))
    json_result = json.loads(fetch_result.text[4:])
    files = json_result["manifest"]["fileRefs"]
    regular_font_filerefs = list(filter(lambda x: "Regular" in x["filename"], files))
    print(min(regular_font_filerefs, key=lambda entry: entry["filename"])["url"])

if __name__ == "__main__":
    for font_name in SAMPLE_FONTS:
        download_regular_version(font_name)
