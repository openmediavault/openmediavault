import { defineConfig } from 'cypress';

export default defineConfig({
  e2e: {
    baseUrl: 'http://localhost:4201',
    fixturesFolder: false,
    setupNodeEvents(on, config) {
      // implement node event listeners here
    }
  },
  env: {
    username: 'admin',
    password: 'openmediavault'
  }
});
