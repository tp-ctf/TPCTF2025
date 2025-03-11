import puppeteer from 'puppeteer';

const sleep = (ms) => new Promise((resolve) => {
  setTimeout(resolve, ms);
});

export default async function visit(url) {
  console.log(`start: ${url}`);

  const browser = await puppeteer.launch({
    headless: 'new',
    executablePath: '/usr/bin/chromium',
    args: [
      '--no-sandbox',
      '--disable-dev-shm-usage',
      '--disable-gpu',
      '--js-flags="--noexpose_wasm"',
      '--disable-extensions-except=/extension',
      '--load-extension=/extension',
    ],
  });

  try {
    const page = await browser.newPage();
    await page.goto(url, { timeout: 5000, waitUntil: 'domcontentloaded' });
    await sleep(5000);
    await page.close();
  } catch (err) {
    console.error(err);
  }

  await browser.close();
  console.log(`end: ${url}`);
}
