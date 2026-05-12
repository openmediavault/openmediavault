describe('Dashboard page', () => {
  beforeEach(() => {
    cy.login();
  });

  it('Is redirected to Dashboard', () => {
    cy.url().should('include', '/dashboard');
  });
});
