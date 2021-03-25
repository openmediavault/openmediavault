export type NavigationBarListItem = {
  text: string;
  icon?: string;
  children?: NavigationBarListItem[];
  url?: string;
  expanded?: boolean;
  active?: boolean;
};
