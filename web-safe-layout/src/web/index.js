import express from 'express';
import session from 'express-session';
import rateLimit from 'express-rate-limit';
import { randomBytes } from 'crypto';
import createDOMPurify from 'dompurify';
import { JSDOM } from 'jsdom';

const { window } = new JSDOM();
const DOMPurify = createDOMPurify(window);

const posts = new Map();

const DEFAULT_LAYOUT = `
<article>
  <h1>Blog Post</h1>
  <div>{{content}}</div>
</article>
`;

const LENGTH_LIMIT = 500;

const app = express();
app.use(express.json());
app.set('view engine', 'ejs');

if (process.env.NODE_ENV === 'production') {
  app.use(
    '/api',
    rateLimit({
      windowMs: 60 * 1000,
      max: 10,
    }),
  );
}

app.use(session({
  secret: randomBytes(32).toString('hex'),
  resave: false,
  saveUninitialized: false,
}));

app.use((req, _, next) => {
  if (!req.session.layouts) {
    req.session.layouts = [DEFAULT_LAYOUT];
    req.session.posts = [];
  }
  next();
});

app.get('/', (req, res) => {
  res.setHeader('Cache-Control', 'no-store');
  res.render('home', {
    posts: req.session.posts,
    maxLayout: req.session.layouts.length - 1,
  });
});

app.post('/api/post', (req, res) => {
  const { content, layoutId } = req.body;
  if (typeof content !== 'string' || typeof layoutId !== 'number') {
    return res.status(400).send('Invalid params');
  }

  if (content.length > LENGTH_LIMIT) return res.status(400).send('Content too long');

  const layout = req.session.layouts[layoutId];
  if (layout === undefined) return res.status(400).send('Layout not found');

  const sanitizedContent = DOMPurify.sanitize(content, { ALLOWED_ATTR: [] });
  const body = layout.replace(/\{\{content\}\}/g, () => sanitizedContent);

  if (body.length > LENGTH_LIMIT) return res.status(400).send('Post too long');

  const id = randomBytes(16).toString('hex');
  posts.set(id, body);
  req.session.posts.push(id);

  console.log(`Post ${id} ${Buffer.from(layout).toString('base64')} ${Buffer.from(sanitizedContent).toString('base64')}`);

  return res.json({ id });
});

app.post('/api/layout', (req, res) => {
  const { layout } = req.body;
  if (typeof layout !== 'string') return res.status(400).send('Invalid param');
  if (layout.length > LENGTH_LIMIT) return res.status(400).send('Layout too large');

  const sanitizedLayout = DOMPurify.sanitize(layout, { ALLOWED_ATTR: [] });

  const id = req.session.layouts.length;
  req.session.layouts.push(sanitizedLayout);
  return res.json({ id });
});

app.get('/post/:id', (req, res) => {
  const { id } = req.params;
  const body = posts.get(id);
  if (body === undefined) return res.status(404).send('Post not found');
  return res.render('post', { id, body });
});

app.post('/api/clear', (req, res) => {
  req.session.layouts = [DEFAULT_LAYOUT];
  req.session.posts = [];
  return res.send('cleared');
});

app.listen(3000, () => {
  console.log('Web server running on port 3000');
});
