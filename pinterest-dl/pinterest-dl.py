#!/usr/bin/env python3

from concurrent.futures import ThreadPoolExecutor
import math
import time
import json
import os
import sys
import requests
import argparse
from datetime import datetime
from bs4 import BeautifulSoup

DEBUG_DIR = "./debug"

BOARD_FEED_RESOURCE = "https://www.pinterest.com/resource/BoardFeedResource/get"
BOARD_SECTION_PINS_RESOURCE = "https://www.pinterest.com/resource/BoardSectionPinsResource/get"

MIN_FILES_PER_THREAD = 10
MAX_THREADS = 30

PROG_NAME = "pinterest-dl"
DESCRIPTION = "bla bla bla"
USAGE = """\
USAGE: {} URL [URL...]

URL - user / board URL\
""".format(PROG_NAME)


def handle_profile(output_dir, info_json):
    username = info_json['resourceResponses'][0]['options']['username']
    print(f"Username: {username}")

    user_dir = f"{output_dir}/{username}"
    if not os.path.isdir(user_dir):
        os.makedirs(user_dir)

    # fetch boards
    boards_info = info_json['resourceResponses'][1]['response']['data']
    for bi in boards_info:
        board_name = bi['name']
        print(f"Fetching board '{board_name}'")
        # fetch board
        board_url = "https://www.pinterest.com" + bi['url']
        handle_board(f"{user_dir}/{board_name}", url=board_url)


def handle_board(output_dir, url=None, info_json=None):
    board = get_board_info(url, info_json)
    if board is None:
        print('failed to fetch board')
        sys.exit(1)
    
    # fetch hanging images
    images = fetch_board_images(BOARD_FEED_RESOURCE, board['url'], {"board_id" : board['id']})
    # fetch sections
    sections = {}
    for s_name, s_id in board.get('sections') or []:
        sections[s_name] = fetch_board_images(BOARD_SECTION_PINS_RESOURCE, board['url'],
                                      {"section_id": s_id})
    # for pretty printing
    l_max = max([len(name) for name in sections])
    print("""\
Board: {}
Hanging Images: {}\n
Sections: \
    """.format(board['name'], len(images)))
    for s_name in sections:
        spaces = ' ' * max(0, l_max - len(s_name))
        print("{}{} - {} images".format(s_name, spaces, len(sections[s_name])))
    print()
    
    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)

    # download hanging images
    print("Downloading board images")
    download_images_mt(images, output_dir)

    # download sections
    for s_name in sections:
        s_images = sections[s_name]
        s_output_dir = f"{output_dir}/{s_name}"
        if not os.path.isdir(s_output_dir):
            os.makedirs(s_output_dir)
        print(f"Downloading section '{s_name}'")
        download_images_mt(s_images, s_output_dir)


def get_board_info(url=None, info_json=None):
    board = None
    # fetch board from url
    if info_json is None and url is not None:
        # TODO: fetch until there are resourceResponses
        board_html = requests.get(url).text
        soup = BeautifulSoup(board_html, "html.parser")

        info_html = soup.find_all("script", attrs={"id": "initial-state"})[0].string
        info_json = json.loads(info_html)
    # parse json
    board = info_json["resourceResponses"][0]["response"]["data"]
    try:
        sections = [
            (section["slug"], section["id"])
            for section in (info_json
                .get("resourceResponses")[2]
                .get("response")
                .get("data")
            )
        ]
        board["sections"] = sections
    except (IndexError, KeyError) as _:
        # Board has no sections!
        pass

    return board


def fetch_board_images(endpoint, board_url, options):
    bookmark = None
    data = []

    while bookmark != '-end-':
        if bookmark:
            options.update({
                "bookmarks": [bookmark]    
            })
        params = {
            "source_url": board_url,
            "data": json.dumps({
                "options": options, 
                "context": {}  
            })
        }
        r = requests.get(endpoint, params=params)
        rj = r.json()
        data += rj["resource_response"]["data"]
        bookmark = rj["resource"]["options"]["bookmarks"][0]

    return data


def download_images(img_data, out_path, n):
    start = time.time()
    # check if out_path is valid
    if not os.path.isdir(out_path):
        os.makedirs(out_path)

    print(f"Thread {n}: Downloading {len(img_data)} images to {out_path}")
    for data in img_data:
        try:
            orig_url = data['images']['orig']['url']
        except:
            print("no 'orig'")
            continue
        name = os.path.basename(orig_url)
        # TODO: check for //
        save_path = f"{out_path}/{name}"
        img = requests.get(orig_url).content
        # save
        with open(save_path, "wb") as f:
            f.write(img)
    print(f"Thread {n}: Done - {time.time() - start}s")


def download_images_mt(images, out_dir):
        print("Simulating download...")
        time.sleep(2)
        return
        # optimal number of threads
        n_threads = math.ceil(len(images) / MIN_FILES_PER_THREAD)
        n_threads = min(n_threads, MAX_THREADS)
        print(f"number of threads: {n_threads}")

        chunk_size = math.ceil(len(images) / n_threads)

        images_chunks = [images[i:i+chunk_size] for i in range(0, len(images), chunk_size)]
        with ThreadPoolExecutor(max_workers=n_threads) as executor:
            executor.map(download_images, images_chunks, [out_dir] * n_threads, range(n_threads))


if __name__ == "__main__":
    # TODO
    # optional args
        # -d download to
        # -v --verbose
        # -i --info - check board (user) info, don't download
    parser = argparse.ArgumentParser(description=DESCRIPTION, prefix_chars="-")
    parser.add_argument("url", metavar="URL", nargs='+')
    parser.add_argument("-d", "--dir", default=os.getcwd(),
                        help="where to download files")
    parser.add_argument("-v", "--verbose", action='store_false', default=False)
    parser.add_argument("-i", "--info", action='store_false', default=False,
                        help="only check profile/board at the URL and print\
info about it (no download)")

    args = parser.parse_args()
    print(args)

    if len(sys.argv) == 1:
        parser.print_usage()
        sys.exit(1)

    # TODO: read args

    # output directory
    output_dir = args.dir
    if args.dir == os.getcwd():
        dt_str = datetime.now().strftime("%d%m%Y-%H:%M:%S")
        output_dir = f"{args.dir}/files{dt_str}"
    # DEBUG
    output_dir = DEBUG_DIR

    if not os.path.isdir(output_dir):
        os.makedirs(output_dir)

    output_dir = f"{DEBUG_DIR}/{dt_str}"
    os.makedirs(output_dir)
    
    url = args.url[0]
    # TODO: is url from pinterest?

    # is url a profile or a board?
    info_json = {}
    req_count = 0
    while 'resourceResponses' not in info_json and req_count < 5:
        html = requests.get(url).text
        soup = BeautifulSoup(html, "html.parser")
        info = soup.find_all("script", attrs={"id": "initial-state"})[0].string
        info_json = json.loads(info)

        req_count += 1

    if 'resourceResponses' not in info_json:
        # do that other thing
        print("no resourceResponses")
        with open('error.json', 'w') as f:
            json.dump(info_json, f, indent=4)
        sys.exit(1)
    else:
        info_name = info_json['resourceResponses'][0]['name']
        if info_name == "LegacyUnauthDesktopUserProfileResource":
            # -- profile --
            handle_profile(output_dir, info_json)

        elif info_name == "BoardResource":
            # -- board --
            board_name = info_json['resourceResponses'][0]['options']['slug']
            handle_board(f"{output_dir}/{board_name}", info_json=info_json)




