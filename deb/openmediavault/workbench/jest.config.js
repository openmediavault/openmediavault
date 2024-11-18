// https://thymikee.github.io/jest-preset-angular/docs
const esModules = ['@codemirror/legacy-modes', 'ansi-regex', 'rxjs', 'strip-ansi'];
module.exports = {
  testTimeout: 10000,
  moduleNameMapper: {
    '~/(.*)$': '<rootDir>/src/$1'
  },
  preset: 'jest-preset-angular',
  setupFiles: ['jest-canvas-mock'],
  setupFilesAfterEnv: ['<rootDir>/src/setupJest.ts'],
  transformIgnorePatterns: ['node_modules/(?!.*\\.mjs$|'.concat(esModules.join('|'), ')')]
};
