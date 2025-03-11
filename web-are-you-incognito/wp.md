https://ouuan.moe/post/2025/03/tpctf-2025

## Are you incognito? (3 solves)

Notice that the bot runs Chromium but the extension uses `browser` instead of `chrome` to access the extension API. It uses `webextension-polyfill` to provide the API under `browser`. We can pass the check if we modify the `browser` object. We cannot directly create JavaScript variables since the web page and the content script run JavaScript separately, but we can use [DOM clobbering](https://domclob.xyz/) to do so.

We need `browser.runtime.id` to pass [the check in `webextension-polyfill`](https://github.com/mozilla/webextension-polyfill/blob/6a42cbeaf637ba3f1283bdcdd657afd06454ca55/src/browser-polyfill.js#L13) and then `browser.extension.inIncognitoContext` to pass the check in the challenge:

```html
<form id="browser" name="runtime"></form>
<form id="browser" name="extension">
  <input name="inIncognitoContext">
</form>
```

An alternative solution is found by USTC-NEBULA, that is to create a global variable `exports` instead of `browser.runtime.id` to pass [the check in `babel-transform-to-umd-module.js`](https://github.com/mozilla/webextension-polyfill/blob/6a42cbeaf637ba3f1283bdcdd657afd06454ca55/scripts/babel-transform-to-umd-module.js#L7).

Some sites such as webhook.site use path instead of subdomain for each user and are thus unable to record the `/flag` request. You can use [requestrepo.com](https://requestrepo.com) or your own server.

This appears to be a 0-day vulnerability[^webextension-polyfill], but I couldn’t find a practical way to exploit it in real-world extensions. I suspect it may at most disrupt the normal execution of content scripts without posing significant security risks or providing strong attack incentives. Anyhow, I will report this to the upstream after the competition. It’s at least a bug if not a vulnerability.

[^webextension-polyfill]: The original version of this challenge was a line `const api = globalThis.browser || globalThis.chrome` instead of the library, but I thought it was too obvious and then found that `webextension-polyfill` was also vulnerable.

P.S. A similar bug was once discovered and fixed in [#153](https://github.com/mozilla/webextension-polyfill/pull/153) but later introduced again in [#582](https://github.com/mozilla/webextension-polyfill/pull/582).
