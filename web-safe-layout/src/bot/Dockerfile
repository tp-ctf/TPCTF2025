FROM guergeiro/pnpm:22-10-slim

RUN apt-get update
RUN apt-get install -y chromium

WORKDIR /app

RUN groupadd -r bot \
    && useradd -r -g bot -G audio,video bot \
    && mkdir -p /home/bot/Downloads \
    && chown -R bot:bot /home/bot

COPY package.json pnpm-lock.yaml ./
ENV PUPPETEER_SKIP_DOWNLOAD=true
RUN pnpm install
COPY . .

USER bot
ENV NODE_ENV=production
CMD ["node", "index.js"]
