require('jest-preset-angular/ngcc-jest-processor');

module.exports = {
  testTimeout: 10000,
  moduleNameMapper: {
    '~/(.*)$': '<rootDir>/src/$1'
  },
  preset: 'jest-preset-angular',
  setupFiles: ['jest-canvas-mock'],
  setupFilesAfterEnv: ['<rootDir>/src/setupJest.ts'],
  testMatch: ['**/*.spec.ts'],
  transformIgnorePatterns: ['/node_modules/(?!ansi-regex|strip-ansi|@codemirror/legacy-modes)']
};
