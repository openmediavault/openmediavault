import { NotificationType } from '~/app/shared/enum/notification-type.enum';

export class Notification {
  private static nextId = 1;

  public readonly id: number;
  public readonly timestamp: string;

  constructor(
    public type: NotificationType = NotificationType.info,
    public title?: string,
    public message?: string,
    public traceback?: string
  ) {
    this.id = Notification.nextId++;
    this.timestamp = new Date().toJSON(); // ISO 8601
  }
}
