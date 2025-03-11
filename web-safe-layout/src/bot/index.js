import express from 'express';
import rateLimit from 'express-rate-limit';
import pLimit from 'p-limit';
import { cpus } from 'os';

import visit from './visit.js';

const app = express();
app.use(express.json());
app.use(express.static('public'));

app.use(
  '/api',
  rateLimit({
    windowMs: 60 * 1000,
    max: 2,
  }),
);

const CONCURRENCY = cpus().length * 2 - 1;
const MAX_PENDING = CONCURRENCY * 2;
const limit = pLimit(CONCURRENCY);

app.post('/api/report', async (req, res) => {
  const { postId } = req.body;
  if (typeof postId !== 'string' || !/^[a-f0-9]{32}$/.test(postId)) {
    return res.status(400).send('Invalid post ID');
  }

  if (limit.pendingCount >= MAX_PENDING) {
    console.log(`Server is busy: ${limit.pendingCount}`);
    return res.status(503).send('Server is busy');
  }

  try {
    await limit(visit, postId);
    return res.sendStatus(200);
  } catch (e) {
    console.error(e);
    return res.status(500).send('Something went wrong');
  }
});

app.listen(1337, () => {
  console.log('Bot server running on port 1337');
});
