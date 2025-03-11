import puppeteer from 'puppeteer';

const FLAG = process.env.FLAG ?? 'fake{dummy}';

const APP_HOST = 'web';
const APP_PORT = '3000';
const APP_URL = `http://${APP_HOST}:${APP_PORT}`;

const sleep = (ms) => new Promise((resolve) => {
  setTimeout(resolve, ms);
});

export default async function visit(postId) {
  const url = new URL(`/post/${postId}`, APP_URL).href;
  console.log(`start: ${url}`);

  const browser = await puppeteer.launch({
    headless: 'new',
    executablePath: '/usr/bin/chromium',
    args: [
      '--no-sandbox',
      '--disable-dev-shm-usage',
      '--disable-gpu',
      '--js-flags="--noexpose_wasm"',
    ],
  });
  const context = await browser.createBrowserContext();
  await context.setCookie({
    name: 'FLAG',
    value: FLAG,
    domain: APP_HOST,
    httpOnly: false,
    sameSite: 'Strict',
  });

  try {
    const page = await context.newPage();
    await page.goto(url, { timeout: 5000, waitUntil: 'domcontentloaded' });
    await sleep(5000);
    await page.close();
  } catch (err) {
    console.error(err);
  }

  await context.close();
  await browser.close();
  console.log(`end: ${url}`);
}
