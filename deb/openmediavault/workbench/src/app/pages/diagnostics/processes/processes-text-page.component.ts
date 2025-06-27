/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
import { Component } from '@angular/core';
import { RpcService } from '~/app/shared/services/rpc.service';
import { finalize } from 'rxjs/operators';

interface Process {
  pid: number;
  user: string;
  pr: number;
  ni: number;
  virt: string;
  res: string;
  shr: string;
  s: string;
  cpu: number;
  mem: number;
  time: string;
  command: string;
}

interface SystemStats {
  loadAverage: {
    oneMin: number;
    fiveMin: number;
    fifteenMin: number;
  };
  memory: {
    total: number;
    used: number;
    free: number;
    buffCache: number;
  };
  swap: {
    total: number;
    used: number;
    free: number;
    available: number;
  };
  uptime: string;
  tasks: {
    total: number;
    running: number;
    sleeping: number;
    stopped: number;
    zombie: number;
  };
}

@Component({
  selector: 'app-processes-page',
  templateUrl: './processes-text-page.component.html',
  styleUrls: ['./processes-text-page.component.scss'],
  standalone: false,
})
export class ProcessesTextPageComponent {
  processes: Process[] = [];
  systemStats: SystemStats | null = null;
  displayedColumns: string[] = ['pid', 'user', 'cpu', 'mem', 'time', 'command'];
  isLoading = false;
  autoRefresh = true;
  refreshInterval = 15000;
  private refreshTimer: any;

  constructor(private rpc: RpcService) {
    this.loadData();
    this.setupAutoRefresh();
  }

loadData() {
  this.isLoading = true;
  this.rpc.request('System', 'getTopInfo', {})
    .pipe(finalize(() => this.isLoading = false))
    .subscribe({
      next: (res: any) => {
        this.parseTopOutput(res);
      },
      error: (err) => {
        console.error('Error loading processes:', err);
      }
    });
}

  private parseTopOutput(data: string) {
    const lines = data.split('\n');
    this.systemStats = this.parseSystemStats(lines);
    this.processes = this.parseProcesses(lines);
  }

private parseSystemStats(lines: string[]): SystemStats {
  const stats: SystemStats = {
    loadAverage: { oneMin: 0, fiveMin: 0, fifteenMin: 0 },
    memory: { total: 0, used: 0, free: 0, buffCache: 0 },
    swap: { total: 0, used: 0, free: 0, available: 0 },
    uptime: '',
    tasks: { total: 0, running: 0, sleeping: 0, stopped: 0, zombie: 0 }
  };

  for (const line of lines) {
    // Parse load average
    if (line.includes('load average:')) {
      const loadMatch = line.match(/load average:\s*(\d+\.\d+),\s*(\d+\.\d+),\s*(\d+\.\d+)/);
      if (loadMatch) {
        stats.loadAverage = {
          oneMin: parseFloat(loadMatch[1]),
          fiveMin: parseFloat(loadMatch[2]),
          fifteenMin: parseFloat(loadMatch[3])
        };
      }
    }
    
    // Parse tasks line
    else if (line.startsWith('Tasks:')) {
      const tasksMatch = line.match(/Tasks:\s*(\d+)\s*total,\s*(\d+)\s*running,\s*(\d+)\s*sleeping,\s*(\d+)\s*stopped,\s*(\d+)\s*zombie/);
      if (tasksMatch) {
        stats.tasks = {
          total: parseInt(tasksMatch[1]),
          running: parseInt(tasksMatch[2]),
          sleeping: parseInt(tasksMatch[3]),
          stopped: parseInt(tasksMatch[4]),
          zombie: parseInt(tasksMatch[5])
        };
      }
    }
    
    // Parse memory (MiB format)
    if (line.includes('MiB Mem :')) {
      const memMatch = line.match(
        /MiB Mem :\s+([\d.]+)\s+total,\s+([\d.]+)\s+free,\s+([\d.]+)\s+used,\s+([\d.]+)\s+buff\/cache/
      );
      if (memMatch) {
        const multiplier = 1024 * 1024; // MiB to bytes
        stats.memory = {
          total: parseFloat(memMatch[1]) * multiplier,
          free: parseFloat(memMatch[2]) * multiplier,
          used: parseFloat(memMatch[3]) * multiplier,
          buffCache: parseFloat(memMatch[4]) * multiplier
        };
      }
    }

    // Parse swap (MiB format)
    if (line.includes('MiB Swap:')) {
      const swapMatch = line.match(
        /MiB Swap:\s+([\d.]+)\s+total,\s+([\d.]+)\s+free,\s+([\d.]+)\s+used\.\s+([\d.]+)\s+avail Mem/
      );
      if (swapMatch) {
        const multiplier = 1024 * 1024; // MiB to bytes
        stats.swap = {
          total: parseFloat(swapMatch[1]) * multiplier,
          free: parseFloat(swapMatch[2]) * multiplier,
          used: parseFloat(swapMatch[3]) * multiplier,
          available: parseFloat(swapMatch[4]) * multiplier
        };
      }
    }
  
    // Parse uptime
    else if (line.startsWith('top -')) {
      const uptimeMatch = line.match(/up\s+(.+?),/);
      if (uptimeMatch) {
        stats.uptime = uptimeMatch[1];
      }
    }
  }

  return stats;
}

  private parseProcesses(lines: string[]): Process[] {
    const processes: Process[] = [];
    
    for (const line of lines) {
      const trimmedLine = line.trim();
      if (trimmedLine && !trimmedLine.startsWith('top') && 
          !trimmedLine.startsWith('Tasks') && !trimmedLine.startsWith('%Cpu') && 
          !trimmedLine.startsWith('KiB') && !trimmedLine.startsWith('load average:')) {
        const parts = trimmedLine.split(/\s+/).filter(p => p);
        if (parts.length >= 12 && !isNaN(parseInt(parts[0]))) {
          processes.push({
            pid: parseInt(parts[0]),
            user: parts[1],
            pr: parseInt(parts[2]),
            ni: parseInt(parts[3]),
            virt: parts[4],
            res: parts[5],
            shr: parts[6],
            s: parts[7],
            cpu: parseFloat(parts[8]),
            mem: parseFloat(parts[9]),
            time: parts[10],
            command: parts.slice(11).join(' ')
          });
        }
      }
    }
    
    return processes;
  }

  toggleAutoRefresh() {
    this.autoRefresh = !this.autoRefresh;
    this.setupAutoRefresh();
  }

  private setupAutoRefresh() {
    if (this.refreshTimer) {
      clearInterval(this.refreshTimer);
    }
    if (this.autoRefresh) {
      this.refreshTimer = setInterval(() => this.loadData(), this.refreshInterval);
    }
  }

  ngOnDestroy() {
    if (this.refreshTimer) {
      clearInterval(this.refreshTimer);
    }
  }

  formatBytes(bytes: number): string {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  }

  calculatePercentage(used: number, total: number): number {
    return total > 0 ? (used / total) * 100 : 0;
  }
}
