describe('Login page', () => {
  beforeEach(() => {
    cy.visit('/');
  });

  it('Dialog exists', () => {
    cy.get('input[type="text"]').should('exist');
    cy.get('input[type="password"]').should('exist');
    cy.get('button[type="submit"]').should('exist');
  });

  it('Log in', () => {
    const username: string = Cypress.env('username');
    const password: string = Cypress.env('password');
    cy.get('[ng-reflect-name="username"]').type(username);
    cy.get('[ng-reflect-name="password"]').type(password);
    cy.get('button[type="submit"]').click();
  });
});
