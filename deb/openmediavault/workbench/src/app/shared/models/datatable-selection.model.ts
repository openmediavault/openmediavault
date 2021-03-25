export class DatatableSelection {
  selected: Array<any> = [];
  hasSelection: boolean;
  hasSingleSelection: boolean;
  hasMultiSelection: boolean;

  constructor() {
    this.update();
  }

  set(selected: any[]) {
    this.selected = selected;
    this.update();
  }

  /**
   * Clear the selection.
   */
  clear() {
    this.selected = [];
    this.update();
  }

  /**
   * Update the internal data structure.
   */
  update() {
    this.hasSelection = this.selected.length > 0;
    this.hasSingleSelection = this.selected.length === 1;
    this.hasMultiSelection = this.selected.length > 1;
  }

  /**
   * Get the first selection.
   */
  first() {
    if (this.hasSelection) {
      return this.selected[0];
    }
    return null;
  }
}
