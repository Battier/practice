#!/usr/bin/env python3
import os
import shutil
import urllib.request
from html.parser import HTMLParser

UPSTREAM_PREFIX = 'https://sonic-jenkins.westus2.cloudapp.azure.com/job/vs/job/buildimage-vs-all/lastSuccessfulBuild/artifact/'

def get_all_bins(target_path, extension):
    """Get all files matching the given extension from the target path"""
    print('Fetching %s*%s' % (target_path, extension))
    os.makedirs(target_path, exist_ok=True)

    req = urllib.request.urlopen(UPSTREAM_PREFIX + target_path)
    data = req.read().decode()

    class Downloader(HTMLParser):
        """Class to parse retrieved data, match against the given extension,
           and download the matching files to the given target directory"""
        def handle_starttag(self, tag, attrs):
            """Handle only <a> tags"""
            if tag == 'a':
                for attr, val in attrs:
                    if attr == 'href' and val.endswith(extension):
                        self.download_file(val)

        @staticmethod
        def download_file(path):
            filename = os.path.join(target_path, path)
            freq = urllib.request.urlopen(UPSTREAM_PREFIX + target_path + path)

            print('\t%s' % path)
            with open(filename, 'wb') as fp:
                shutil.copyfileobj(freq, fp)


    parser = Downloader()
    parser.feed(data)
    print()

get_all_bins('target/debs/stretch/', '.deb')
get_all_bins('target/files/stretch/', '.ko')
get_all_bins('target/python-debs/', '.deb')
get_all_bins('target/python-wheels/', '.whl')
get_all_bins('target/', '.gz')
