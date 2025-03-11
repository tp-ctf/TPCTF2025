if (browser.extension.inIncognitoContext) {
  fetch('/flag', {
    method: 'POST',
    body: 'fake{dummy}',
  });
} else {
  console.log('No flag for you!');
}
