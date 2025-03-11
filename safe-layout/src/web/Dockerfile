FROM guergeiro/pnpm:22-10-slim
WORKDIR /app

COPY package.json pnpm-lock.yaml ./
RUN pnpm install
COPY . .

USER 404:404
ENV NODE_ENV=production
CMD ["node", "index.js"]
