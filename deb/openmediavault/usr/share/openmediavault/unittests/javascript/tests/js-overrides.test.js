require('../../../../../../var/www/openmediavault/js/js-overrides.js');

describe('String', () => {
	it('Should convert special HTML entities', () => {
		expect('foo&bar'.htmlspecialchars()).toBe('foo&amp;bar');
		expect('\'test"'.htmlspecialchars()).toBe('&#039;test&quot;');
		expect('1 < 0'.htmlspecialchars()).toBe('1 &lt; 0');
		expect('1 > 0'.htmlspecialchars()).toBe('1 &gt; 0');
	});

	it('Should rtrim the string', () => {
		expect('\n\n test \n\n'.rtrim(' \n')).toBe('\n\n test');
	});

	it('Should ltrim the string', () => {
		expect('\n\n test \n\n'.ltrim(' \n')).toBe('test \n\n');
	});

	it('Should trim backslashes', () => {
		expect('\\\\foo'.ltrim('\\')).toBe('foo');
		expect('bar\\\\'.rtrim('\\')).toBe('bar');
	});

	it('Should trim the string', () => {
		expect('\n\ntest\n\n'.lrtrim('\n')).toBe('test');
	});
});

describe('Date', () => {
	it('Should have mapHour', () => {
		expect(Array.isArray(Date.mapHour)).toBeTruthy();
	});

	it('Should have mapHour2Digits', () => {
		expect(Array.isArray(Date.mapHour2Digits)).toBeTruthy();
	});

	it('Should have mapDayOfWeek', () => {
		expect(Array.isArray(Date.mapDayOfWeek)).toBeTruthy();
	});

	it('Should have mapDayOfMonth', () => {
		expect(Array.isArray(Date.mapDayOfMonth)).toBeTruthy();
	});

	it('Should have mapDayOfMonth2Digits', () => {
		expect(Array.isArray(Date.mapDayOfMonth2Digits)).toBeTruthy();
	});

	it('Should have mapMonth', () => {
		expect(Array.isArray(Date.mapMonth)).toBeTruthy();
	});

	it('Should have mapMonth2Digits', () => {
		expect(Array.isArray(Date.mapMonth2Digits)).toBeTruthy();
	});

	it('Should convert to unix timestamp', () => {
		var dt = new Date('December 17, 1995 03:24:00');
		expect(dt.toUnixTimestamp()).toEqual(819167040);
	});
});

describe('Number', () => {
	it('Should convert from KiB to MiB', () => {
		var value = 2048;
		expect(value.binaryConvert('KiB', 'MiB')).toBe(2);
	});

	it('Should convert from GiB to MiB', () => {
		var value = 4;
		expect(value.binaryConvert('GiB', 'MiB')).toBe(4096);
	});

	it('Should convert to MiB', () => {
		var value = 2097152;
		expect(value.binaryFormat()).toBe('2.00 MiB');
	});

	it('Should convert to KiB', () => {
		var value = 2097152;
		expect(value.binaryFormat({maxUnit: 'KiB'})).toBe('2048.00 KiB');
	});

	it('Should convert to KiB (no decimal places)', () => {
		var value = 4096;
		expect(value.binaryFormat({decimalPlaces: 0})).toBe('4 KiB');
	});

	it('Should convert to KiB (3 decimal places)', () => {
		var value = 2048;
		expect(value.binaryFormat({decimalPlaces: 3})).toBe('2.000 KiB');
	});

	it('Should convert to KiB (indexed)', () => {
		var value = 4096;
		expect(value.binaryFormat({indexed: true})).toEqual({
			'divisor': 1024,
			'exponent': 1,
			'unit': 'KiB',
			'value': '4.00'
		});
	});
});
