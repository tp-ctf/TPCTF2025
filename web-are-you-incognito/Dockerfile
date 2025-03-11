FROM guergeiro/pnpm:22-10-slim

RUN apt-get update
RUN apt-get install -y chromium

WORKDIR /extension
COPY extension .
RUN pnpm install

WORKDIR /app

RUN groupadd -r bot \
    && useradd -r -g bot -G audio,video bot \
    && mkdir -p /home/bot/Downloads \
    && chown -R bot:bot /home/bot

COPY bot/package.json bot/pnpm-lock.yaml ./
ENV PUPPETEER_SKIP_DOWNLOAD=true
RUN pnpm install
COPY bot .

USER bot
ENV NODE_ENV=production
CMD ["node", "index.js"]
