/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
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
import 'cronstrue/locales/de';
import 'cronstrue/locales/ru';
import 'cronstrue/locales/nl';
import 'cronstrue/locales/it';
import 'cronstrue/locales/tr';
import 'cronstrue/locales/fr';
import 'cronstrue/locales/pl';
import 'cronstrue/locales/cs';
import 'cronstrue/locales/es';
import 'cronstrue/locales/ca';
import 'cronstrue/locales/hu';
import 'cronstrue/locales/da';
import 'cronstrue/locales/uk';
import 'cronstrue/locales/sv';
import 'cronstrue/locales/pt_PT';
import 'cronstrue/locales/sl';
import 'cronstrue/locales/zh_CN';
import 'cronstrue/locales/zh_TW';
import 'cronstrue/locales/ja';
import 'cronstrue/locales/ko';
import 'cronstrue/locales/ar';

import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';
import cronstrue from 'cronstrue';
import dayjs from 'dayjs';
import advancedFormat from 'dayjs/plugin/advancedFormat';
import relativeTime from 'dayjs/plugin/relativeTime';
import * as _ from 'lodash';
import * as nunjucks from 'nunjucks';
import validator from 'validator';

import { getCurrentLocale, translate } from '~/app/i18n.helper';

//////////////////////////////////////////////////////////////////////////
// Initialize additional dayjs plugins.
dayjs.extend(advancedFormat);
dayjs.extend(relativeTime);

//////////////////////////////////////////////////////////////////////////
// Create Nunjucks environment.
const nunjucksEnv = new nunjucks.Environment();

/**
 * Convert a value to a boolean.
 *
 * @param value The value to convert.
 * @return Returns `true` if the value represents true,
 *   otherwise `false`.
 */
export const toBoolean = (value: any): boolean => {
  let result = false;
  switch (value) {
    case true:
    case 1:
    case 'y':
    case 'Y':
    case 'yes':
    case 'Yes':
    case 'YES':
    case 't':
    case 'true':
    case 'True':
    case 'TRUE':
    case 'on':
    case 'On':
    case 'ON':
    case 'ok':
    case 'Ok':
    case 'OK':
    case '1':
      result = true;
      break;
  }
  return result;
};

/**
 * Convert a boolean value to string.
 *
 * @param value The boolean value to convert.
 * @return Returns `Yes` if the value represents true,
 *   otherwise `No`.
 */
export const yesNo = (value: any): string => (toBoolean(value) ? gettext('Yes') : gettext('No'));

/**
 * Render a Nunjucks/Jinja2 template.
 *
 * @param str The template to render.
 * @param data The object containing the data to replace
 *   the tokens.
 * @return Returns the rendered template.
 */
export const renderTemplate = (str: string, data: Record<any, any>): string => {
  const template = nunjucks.compile(str, nunjucksEnv);
  return template.render(data);
};

/**
 * Checks if value is can be formatted.
 *
 * @param value The value to check.
 * @return Returns `true` if value can be formatted, otherwise `false`.
 */
export const isFormatable = (value: any): boolean =>
  _.isString(value) || _.isPlainObject(value) || _.isArray(value);

/**
 * Allows you to define a tokenized string and pass an object
 * to replace the tokens.
 *
 * Note, output with dangerous characters is escaped automatically.
 * Use the `safe` filter to bypass this behaviour.
 *
 * @param str The string to format.
 *   Examples:
 *     '/storage/disks/edit/{{ hdparm.uuid }}'
 *     'Hello, my name is {{ name }}.'
 *     'Current count is: {{ count }}'
 *     '{{ qux[1] }}'
 * @param data The object containing the data to replace
 *   the tokens.
 *   Example:
 *     { hdparm: { uuid: 'xxx}, name: 'Hugo', count: 2, qux: ['a', 'b'] }
 * @return Returns the formatted string.
 */
export const format = (str: string, data: Record<any, any>): string => renderTemplate(str, data);

/**
 * Format all tokenized string values recursively in the given
 * object.
 *
 * Note, output with dangerous characters is escaped automatically.
 * Use the `safe` filter to bypass this behaviour.
 *
 * @param value The object to format.
 * @param data The object containing the data to replace.
 * @return Returns a deep copy of the given object with the
 *   modified string values.
 */
export const formatDeep = (value: any, data: Record<any, any>): Record<any, any> => {
  const fn = (m) => {
    if (_.isPlainObject(m) || _.isArray(m)) {
      _.forEach(m, (v, k) => {
        m[k] = fn(v);
      });
    } else if (_.isString(m)) {
      m = format(m, data);
    }
    return m;
  };
  return fn(_.cloneDeep(value));
};

/**
 * Allows you to define a tokenized URI and pass an object
 * to replace the tokens. The URI parameters will be encoded.
 *
 * Note, output with dangerous characters is escaped automatically.
 * Use the `safe` filter to bypass this behaviour.
 *
 * @param uri The URI to format.
 * @param params The object containing the parameters to replace
 *   the tokens.
 * @return Returns the formatted URI.
 */
export const formatURI = (uri: string, params: Record<any, any>): string =>
  format(uri, encodeURIComponentDeep(params));

/**
 * Encode all string values recursively in the given object.
 *
 * @param o The object to be processed.
 * @return Returns a deep copy of the given object with the
 *   modified encoded values.
 */
export const encodeURIComponentDeep = (o: Record<any, any> | undefined): Record<any, any> => {
  const fn = (m) => {
    if (_.isPlainObject(m) || _.isArray(m)) {
      _.forEach(m, (v, k) => {
        m[k] = fn(v);
      });
    } else if (_.isString(m)) {
      m = encodeURIComponent(m);
    }
    return m;
  };
  return fn(_.cloneDeep(o));
};

/**
 * Decode all string values recursively in the given object.
 *
 * @param o The object to be processed.
 * @return Returns a deep copy of the given object with the
 *   modified decoded values.
 */
export const decodeURIComponentDeep = (o: Record<any, any> | undefined): Record<any, any> => {
  const fn = (m) => {
    if (_.isPlainObject(m) || _.isArray(m)) {
      _.forEach(m, (v, k) => {
        m[k] = fn(v);
      });
    } else if (_.isString(m)) {
      m = decodeURIComponent(m);
    }
    return m;
  };
  return fn(_.cloneDeep(o));
};

/**
 * Check whether a variable is an UUIDv4.
 *
 * @deprecated
 * @param value The variable being evaluated.
 * @return `true` if the variable is an UUIDv4, otherwise `false`.
 */
export const isUUIDv4 = (value: any): boolean => isUUID(value);

/**
 * Check whether a variable is a UUID.
 *
 * @param value The variable being evaluated.
 * @param version The UUID version.
 * @return `true` if the variable is a UUID, otherwise `false`.
 */
export const isUUID = (value: any, version?: validator.UUIDVersion): boolean => {
  if (!_.isString(value) || _.isEmpty(value)) {
    return false;
  }
  return validator.isUUID(value, version);
};

/**
 * Check whether a variable is an UUIDv4 that represents a new database
 * configuration object.
 *
 * @param value The variable being evaluated.
 * @return Returns `true` if the UUIDv4 represents the identifier of a
 *   new database configuration object, otherwise `false`.
 */
export const isNewConfObjUuid = (value: any): boolean => {
  if (!isUUID(value)) {
    return false;
  }
  return value === format('{{ newconfobjuuid }}', {});
};

/**
 * Convert a binary value into bytes.
 *
 * @param value The value to be converted, e.g. 512, 2048B or 2.50 MiB.
 * @returns Returns the given value as number in bytes without any unit
 *   appended or an empty string in case of an error.
 */
export const toBytes = (value: string | number): number | string => {
  const base = 1024;
  const units = ['b', 'k', 'm', 'g', 't', 'p', 'e', 'z', 'y'];
  const matches = RegExp('^(\\d+(.\\d+)?)\\s?([' + units.join('') + ']?(b|ib|B/s)?)?$', 'i').exec(
    String(value)
  );
  if (_.isNull(matches)) {
    return '';
  }
  let bytes = parseFloat(matches[1]);
  if (_.isString(matches[3])) {
    bytes = bytes * Math.pow(base, units.indexOf(matches[3].toLowerCase()[0]));
  }
  return Math.round(bytes);
};

/**
 * Convert a number of bytes into the highest possible binary unit.
 * Example: 2097152 will become 2.00 MiB
 *
 * @param value The number of bytes to be converted.
 * @param fractionDigits The number of digits after the decimal point.
 *   Defaults to 2.
 * @param maxUnit The max. unit to be used, e.g. 'KiB' or 'MiB'.
 * @return The converted value, e.g. '2.50 MiB'. Negative values will
 *   return an empty string. A value of 0 will return a '0' string
 *   without a unit.
 */
export const binaryUnit = (
  value: string | number,
  fractionDigits?: number,
  maxUnit?: string
): string => {
  const prefixes = ['B', 'KiB', 'MiB', 'GiB', 'TiB', 'PiB', 'EiB', 'ZiB', 'YiB'];
  let curExp = 0;
  let maxExp = prefixes.length;
  let numberValue = value;

  if (_.isString(numberValue)) {
    // Convert to number, empty strings will become NaN.
    numberValue = _.parseInt(numberValue);
  }

  if (!_.isFinite(numberValue)) {
    return String(value);
  }

  // Negative values do not make sense.
  if (numberValue < 0) {
    return '';
  }

  fractionDigits = _.isNumber(fractionDigits) ? fractionDigits : 2;
  if (_.isString(maxUnit) && !_.isEmpty(maxUnit)) {
    maxExp = prefixes.indexOf(maxUnit);
  }

  while (numberValue > 1024 && curExp < maxExp) {
    curExp++;
    numberValue = numberValue / 1024;
  }

  return `${numberValue.toFixed(fractionDigits)} ${prefixes[curExp]}`;
};

/**
 * Return the default value if the input value is undefined, null,
 * NaN or empty.
 *
 * @param value The value to process.
 * @param defaultValue The default value, defaults to 'n/a'.
 * @return The input value or the given default value if the specified
 *   conditions do not match.
 */
export const notAvailable = (value, defaultValue?: any) =>
  _.isUndefined(value) || _.isNull(value) || _.isNaN(value) || value === ''
    ? _.defaultTo(defaultValue, translate(gettext('n/a')))
    : value;

/**
 * Get the Unix time stamp from the given time in milliseconds.
 *
 * @param ms The time in milliseconds. Defaults to `Date.now()`.
 * @return Returns the Unix time stamp (in seconds) from the given time.
 */
export const unixTimeStamp = (ms: number = Date.now()): number => Math.floor(ms / 1000);

/**
 * Convert the specified date/time appropriate to the host environment's
 * current locale or into relative time like '2 hours ago'.
 *
 * @param value The date/time value. If it is a number, a UNIX epoch
 *   timestamp is assumed.
 * @param dateFormat The format to use, e.g. 'date', 'time', 'datetime'
 *   or 'relative'. Defaults to 'date'.
 * @params options Additional options.
 * @return The time in the given format or an empty string on failure.
 */
export const toLocaleDate = (
  value: Date | string | number,
  dateFormat?: 'relative' | 'datetime' | 'time' | 'date',
  options?: any
): any => {
  if (!(_.isString(value) || _.isDate(value) || _.isNumber(value))) {
    return '';
  }
  let date: dayjs.Dayjs;
  if (_.isNumber(value)) {
    date = dayjs.unix(value);
  } else {
    date = dayjs(value);
  }
  if (!date.isValid()) {
    return '';
  }
  let result;
  switch (dateFormat) {
    case 'relative':
      result = date.fromNow(options);
      break;
    default:
      result = dateToLocale(date.toDate(), dateFormat);
      break;
  }
  return result;
};

/**
 * Helper function to convert a `Date` object to the given format.
 * The method is outsourced mainly for mocking issues in unit tests.
 */
export const dateToLocale = (date: Date, dateFormat?: 'datetime' | 'time' | 'date'): string => {
  let result;
  switch (dateFormat) {
    case 'datetime':
      result = date.toLocaleString();
      break;
    case 'time':
      result = date.toLocaleTimeString();
      break;
    default:
    case 'date':
      result = date.toLocaleDateString();
      break;
  }
  return result;
};

/**
 * Convert a Cron expression into a human-readable description.
 *
 * @param value The Cron expression, e.g. `5 13 * * 5`.
 * @return The Cron expression as human-readable description.
 */
export const cron2human = (value: string): string => {
  // These are the locales that are supported by the upstream cRonstrue
  // project. Map the internal locale short codes to the codes that are
  // used by the library.
  const localeMap = {
    /* eslint-disable @typescript-eslint/naming-convention */
    ar_SA: 'ar', // Arabic
    ca_ES: 'ca', // Catalan
    cs_CZ: 'cs', // Czech
    da_DA: 'da', // Danish
    de_DE: 'de', // German
    es_ES: 'es', // Spanish
    fr_FR: 'fr', // French
    hu_HU: 'hu', // Hungarian
    it_IT: 'it', // Italian
    ja_JP: 'ja', // Japanese (Japan)
    ko_KR: 'ko', // Korean
    nl_NL: 'nl', // Dutch
    pl_PL: 'pl', // Polish
    pt_PT: 'pt_PT', // Portuguese
    ru_RU: 'ru', // Russian
    sl_SI: 'sl', // Slovenian
    sv_SV: 'sv', // Swedish
    tr_TR: 'tr', // Turkish
    uk_UK: 'uk', // Ukrainian
    zh_CN: 'zh_CN', // Chinese (Simplified Chinese)
    zh_TW: 'zh_TW' // Chinese (Taiwan)
    /* eslint-enable @typescript-eslint/naming-convention */
  };
  return cronstrue.toString(value, {
    locale: _.get(localeMap, getCurrentLocale(), 'en')
  });
};

//////////////////////////////////////////////////////////////////////////
// Add custom Nunjucks filter.
/**
 * Format the current time using the given format.
 */
nunjucksEnv.addGlobal('moment', (fmt: string = 'HH:mm:ss') => dayjs().format(fmt));
nunjucksEnv.addGlobal('location', () => location);
/**
 * This is the UUID that is used to inform the backend that the database
 * configuration object currently processed needs to become a new unique
 * identifier when it is committed into the database. This is a stupid
 * behaviour from the early beginning of OMV.
 */
nunjucksEnv.addGlobal('newconfobjuuid', 'fa4b1c66-ef79-11e5-87a0-0002b3a176b4');
/**
 * Translate the given string.
 */
nunjucksEnv.addFilter('translate', translate);
nunjucksEnv.addFilter('binaryunit', binaryUnit);
nunjucksEnv.addFilter('tobytes', toBytes);
/**
 * Formats a number using fixed-point notation. If the given value is not a number,
 * then `NaN` is returned.
 */
nunjucksEnv.addFilter('tofixed', (value: number, fractionDigits: number = 0) =>
  _.isNumber(value) ? value.toFixed(fractionDigits) : NaN
);
nunjucksEnv.addFilter('min', (value: number, minValue: number) => Math.min(value, minValue));
nunjucksEnv.addFilter('max', (value: number, maxValue: number) => Math.max(value, maxValue));
nunjucksEnv.addFilter('notavailable', notAvailable);
/**
 * Convert an object to a boolean.
 */
nunjucksEnv.addFilter('toboolean', toBoolean);
nunjucksEnv.addFilter('not', (value: any) => !value);
nunjucksEnv.addFilter('yesno', yesNo);
nunjucksEnv.addFilter('strip', (str: string, chars?: string) => _.trim(str, chars));
nunjucksEnv.addFilter('lstrip', (str: string, chars?: string) => _.trimStart(str, chars));
nunjucksEnv.addFilter('rstrip', (str: string, chars?: string) => _.trimEnd(str, chars));
nunjucksEnv.addFilter('get', (data: any, path: number, defaultValue?: any) =>
  _.get(data, path, defaultValue)
);
nunjucksEnv.addFilter('repeat', (str: string, n: number = 1) => _.repeat(str, n));
nunjucksEnv.addFilter('split', (str: string, separator?: string, limit?: number) =>
  str.split(separator, limit)
);
nunjucksEnv.addFilter('substr', (str: string, from: number, length?: number) =>
  str.substr(from, length)
);
nunjucksEnv.addFilter('basename', (str: string) => {
  const parts = str.split('/');
  return _.last(parts);
});
nunjucksEnv.addFilter('keys', (value: Record<string, any>) => _.keys(value));
nunjucksEnv.addFilter('array', (value: any) => (!_.isArray(value) ? [value] : value));
nunjucksEnv.addFilter('includes', (collection: Array<any>, value: any) =>
  _.includes(collection, value)
);
nunjucksEnv.addFilter('isequal', (value: any, other: any) => _.isEqual(value, other));
nunjucksEnv.addFilter('union', (value: Array<any>, other: Array<any>) => _.union(value, other));
nunjucksEnv.addFilter('uniq', (value: Array<any>) => _.uniq(value));
nunjucksEnv.addFilter('map', (value: Array<any>, filter: string, ...filterArgs: any) => {
  const filterFn = nunjucksEnv.getFilter(filter);
  return _.map(value, (part) => filterFn.apply(this, [part, ...filterArgs]));
});
/**
 * Note, this will override the built-in `slice` filter.
 */
nunjucksEnv.addFilter('slice', (value: Array<any>, start?: number, end?: number) =>
  value.slice(start, end)
);
/**
 * Creates an array of own enumerable string keyed-value pairs.
 */
nunjucksEnv.addFilter('entries', (value: Record<string, any>) => {
  const x = _.entries(value);
  return x;
});
/**
 * Creates an array with all falsey values removed. The values false, null,
 * 0, "", undefined, and NaN are falsey.
 */
nunjucksEnv.addFilter('compact', (value: Array<any>) => _.compact(value));
/**
 * Encodes a text string as a valid component of a Uniform Resource
 * Identifier (URI).
 */
nunjucksEnv.addFilter('encodeuricomponent', (value: string | number | boolean): string =>
  encodeURIComponent(value)
);
/**
 * Convert the value to the host environment's current locale or
 * into relative time.
 */
nunjucksEnv.addFilter('localedate', toLocaleDate);
/**
 * Convert a Cron expression into a human-readable description.
 */
nunjucksEnv.addFilter('cron2human', cron2human);
