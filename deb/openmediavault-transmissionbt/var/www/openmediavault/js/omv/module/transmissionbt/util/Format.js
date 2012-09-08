/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 28.11.11
 * Time: 21:12
 * To change this template use File | Settings | File Templates.
 */

Ext.ns("OMV.Module.Services.TransmissionBT.util");

OMV.Module.Services.TransmissionBT.util.Format = function() {
	var f = function() {};
	f.prototype = OMV.util.Format;
	var o = function() {};
	Ext.extend(o, f, function() {
		return {
			bytesToSize : function(bytes) {
				var sizes = ['Bytes', 'KiB', 'MiB', 'GiB', 'TiB', 'PiB', 'EiB', 'ZiB', 'YiB'];
				if (bytes == 0) return 'n/a';
				var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
				return ((i == 0)? (bytes / Math.pow(1024, i)) : (bytes / Math.pow(1024, i)).toFixed(1)) + ' ' + sizes[i];
			},

			timeInterval : function(seconds) {
				var weeks    = Math.floor (seconds / 604800),
						days    = Math.floor ((seconds % 604800) / 86400),
						hours   = Math.floor ((seconds % 86400) / 3600),
						minutes = Math.floor ((seconds % 3600) / 60),
						secondsLeft = Math.floor (seconds % 60),
						w = weeks   + 'w',
						d = days    + 'd',
						h = hours   + 'h',
						m = minutes + 'm',
						s = secondsLeft + 's';

				if (weeks) {
					return w + ' ' + d;
				}
				if (days) {
					return d + ' ' + h;
				}
				if (hours) {
					return h + ' ' + m;
				}
				if (minutes) {
					return m + ' ' + s;
				}
				return s;
			},

			rate : function(Bps) {
				var speed = Math.floor(Bps / 1000);

				if (speed <= 999.95) // 0 KBps to 999 K
					return [ speed.toTruncFixed(0), 'KB/s' ].join(' ');

				speed /= 1000;

				if (speed <= 99.995) // 1 M to 99.99 M
					return [ speed.toTruncFixed(2), 'MB/s' ].join(' ');
				if (speed <= 999.95) // 100 M to 999.9 M
					return [ speed.toTruncFixed(1), 'MB/s' ].join(' ');

				// insane speeds
				speed /= 1000;
				return [ speed.toTruncFixed(2), 'GB/s' ].join(' ');
			}
		};
	}());
	return new o();
}();

Number.prototype.toTruncFixed = function(place) {
        var ret = Math.floor(this * Math.pow (10, place)) / Math.pow(10, place);
        return ret.toFixed(place);
};

Number.prototype.toStringWithCommas = function() {
    return this.toString().replace(/\B(?=(?:\d{3})+(?!\d))/g, ",");
};