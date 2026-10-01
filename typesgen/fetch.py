import requests


def fetch(url):
    r = requests.get(url)
    return r.text

def get_asset(file):
    url = f"https://files.sc-workshop.com/asset-request?type=downloadAssets&request={file}&index=0&size=30&compressed=false"
    return fetch(url)
