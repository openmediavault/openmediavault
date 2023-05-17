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
    cy.get('[ng-reflect-name="username"]').type(Cypress.env('username'));
    cy.get('[ng-reflect-name="password"]').type(Cypress.env('password'));
    cy.get('button[type="submit"]').click();
  });
});
