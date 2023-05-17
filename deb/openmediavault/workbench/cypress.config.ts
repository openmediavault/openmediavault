import { defineConfig } from 'cypress';

export default defineConfig({
  e2e: {
    baseUrl: 'http://localhost:4201',
    fixturesFolder: false
  },
  env: {
    username: 'test',
    password: 'test'
  }
});
