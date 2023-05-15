const esModules = ['@codemirror/legacy-modes', 'ansi-regex', 'rxjs', 'strip-ansi'];
module.exports = {
  testTimeout: 10000,
  moduleNameMapper: {
    '~/(.*)$': '<rootDir>/src/$1'
  },
  preset: 'jest-preset-angular',
  globalSetup: 'jest-preset-angular/global-setup',
  setupFiles: ['jest-canvas-mock'],
  setupFilesAfterEnv: ['<rootDir>/src/setupJest.ts'],
  testMatch: ['**/*.spec.ts'],
  transformIgnorePatterns: ['node_modules/(?!.*\\.mjs$|'.concat(esModules.join('|'), ')')],
  moduleFileExtensions: ['ts', 'html', 'js', 'json', 'mjs']
};
