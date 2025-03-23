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
import { Component } from '@angular/core';
import { marker as gettext } from '@ngneat/transloco-keys-manager/marker';

import { FormPageConfig } from '~/app/core/components/intuition/models/form-page-config.type';
import { BaseFormPageComponent } from '~/app/pages/base-page-component';

@Component({
  template: '<omv-intuition-form-page [config]="this.config"></omv-intuition-form-page>'
})
export class SslCertificateFormPageComponent extends BaseFormPageComponent {
  public config: FormPageConfig = {
    request: {
      service: 'CertificateMgmt',
      post: {
        method: 'create'
      }
    },
    fields: [
      {
        type: 'confObjUuid'
      },
      {
        type: 'select',
        name: 'size',
        label: gettext('Key size'),
        hint: gettext('The RSA key length.'),
        value: 4096,
        store: {
          data: [
            {
              text: '2048b',
              value: 2048
            },
            {
              text: '4096b',
              value: 4096
            }
          ],
          sorters: [
            {
              dir: 'asc',
              prop: 'value'
            }
          ]
        }
      },
      {
        type: 'select',
        name: 'days',
        label: gettext('Period of validity'),
        hint: gettext('The number of days the certificate is valid for.'),
        value: 365,
        store: {
          data: [
            {
              text: gettext('1 day'),
              value: 1
            },
            {
              text: gettext('2 days'),
              value: 2
            },
            {
              text: gettext('3 days'),
              value: 3
            },
            {
              text: gettext('4 days'),
              value: 4
            },
            {
              text: gettext('5 days'),
              value: 5
            },
            {
              text: gettext('6 days'),
              value: 6
            },
            {
              text: gettext('1 week'),
              value: 7
            },
            {
              text: gettext('2 weeks'),
              value: 14
            },
            {
              text: gettext('3 weeks'),
              value: 21
            },
            {
              text: gettext('1 month'),
              value: 30
            },
            {
              text: gettext('3 months'),
              value: 90
            },
            {
              text: gettext('6 months'),
              value: 180
            },
            {
              text: gettext('9 months'),
              value: 270
            },
            {
              text: gettext('1 year'),
              value: 365
            },
            {
              text: gettext('2 years'),
              value: 740
            },
            {
              text: gettext('3 years'),
              value: 1095
            },
            {
              text: gettext('4 years'),
              value: 1460
            },
            {
              text: gettext('5 years'),
              value: 1825
            },
            {
              text: gettext('10 years'),
              value: 3650
            },
            {
              text: gettext('15 years'),
              value: 5475
            },
            {
              text: gettext('20 years'),
              value: 7300
            },
            {
              text: gettext('25 years'),
              value: 9125
            }
          ],
          sorters: [
            {
              dir: 'asc',
              prop: 'value'
            }
          ]
        }
      },
      {
        type: 'textInput',
        name: 'cn',
        value: '{{ location() | get("hostname") }}',
        label: gettext('Common Name'),
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'o',
        value: '',
        label: gettext('Organization Name')
      },
      {
        type: 'textInput',
        name: 'ou',
        value: '',
        label: gettext('Organization Unit')
      },
      {
        type: 'textInput',
        name: 'l',
        value: '',
        label: gettext('City')
      },
      {
        type: 'textInput',
        name: 'st',
        value: '',
        label: gettext('State/Province')
      },
      {
        type: 'select',
        name: 'c',
        value: '',
        label: gettext('Country'),
        placeholder: gettext('Select a country ...'),
        store: {
          data: [
            { value: 'AF', text: gettext('Afghanistan') },
            { value: 'AX', text: gettext('Aland Islands') },
            { value: 'AL', text: gettext('Albania') },
            { value: 'DZ', text: gettext('Algeria') },
            { value: 'AS', text: gettext('American Samoa') },
            { value: 'AD', text: gettext('Andorra') },
            { value: 'AO', text: gettext('Angola') },
            { value: 'AI', text: gettext('Anguilla') },
            { value: 'AQ', text: gettext('Antarctica') },
            { value: 'AG', text: gettext('Antigua and Barbuda') },
            { value: 'AR', text: gettext('Argentina') },
            { value: 'AM', text: gettext('Armenia') },
            { value: 'AW', text: gettext('Aruba') },
            { value: 'AU', text: gettext('Australia') },
            { value: 'AT', text: gettext('Austria') },
            { value: 'AZ', text: gettext('Azerbaijan') },
            { value: 'BS', text: gettext('Bahamas') },
            { value: 'BH', text: gettext('Bahrain') },
            { value: 'BD', text: gettext('Bangladesh') },
            { value: 'BB', text: gettext('Barbados') },
            { value: 'BY', text: gettext('Belarus') },
            { value: 'BE', text: gettext('Belgium') },
            { value: 'BZ', text: gettext('Belize') },
            { value: 'BJ', text: gettext('Benin') },
            { value: 'BM', text: gettext('Bermuda') },
            { value: 'BT', text: gettext('Bhutan') },
            { value: 'BO', text: gettext('Bolivia') },
            { value: 'BA', text: gettext('Bosnia and Herzegovina') },
            { value: 'BW', text: gettext('Botswana') },
            { value: 'BV', text: gettext('Bouvet Island') },
            { value: 'BR', text: gettext('Brazil') },
            { value: 'BQ', text: gettext('British Antarctic Territory') },
            { value: 'IO', text: gettext('British Indian Ocean Territory') },
            { value: 'VG', text: gettext('British Virgin Islands') },
            { value: 'BN', text: gettext('Brunei') },
            { value: 'BG', text: gettext('Bulgaria') },
            { value: 'BF', text: gettext('Burkina Faso') },
            { value: 'BI', text: gettext('Burundi') },
            { value: 'KH', text: gettext('Cambodia') },
            { value: 'CM', text: gettext('Cameroon') },
            { value: 'CA', text: gettext('Canada') },
            { value: 'CT', text: gettext('Canton and Enderbury Islands') },
            { value: 'CV', text: gettext('Cape Verde') },
            { value: 'KY', text: gettext('Cayman Islands') },
            { value: 'CF', text: gettext('Central African Republic') },
            { value: 'TD', text: gettext('Chad') },
            { value: 'CL', text: gettext('Chile') },
            { value: 'CN', text: gettext('China') },
            { value: 'CX', text: gettext('Christmas Island') },
            { value: 'CC', text: gettext('Cocos (Keeling) Islands') },
            { value: 'CO', text: gettext('Colombia') },
            { value: 'KM', text: gettext('Comoros') },
            { value: 'CG', text: gettext('Congo (Brazzaville)') },
            { value: 'CD', text: gettext('Congo (Kinshasa)') },
            { value: 'CK', text: gettext('Cook Islands') },
            { value: 'CR', text: gettext('Costa Rica') },
            { value: 'HR', text: gettext('Croatia') },
            { value: 'CU', text: gettext('Cuba') },
            { value: 'CY', text: gettext('Cyprus') },
            { value: 'CZ', text: gettext('Czech Republic') },
            { value: 'DK', text: gettext('Denmark') },
            { value: 'DJ', text: gettext('Djibouti') },
            { value: 'DM', text: gettext('Dominica') },
            { value: 'DO', text: gettext('Dominican Republic') },
            { value: 'NQ', text: gettext('Dronning Maud Land') },
            { value: 'TL', text: gettext('East Timor') },
            { value: 'EC', text: gettext('Ecuador') },
            { value: 'EG', text: gettext('Egypt') },
            { value: 'SV', text: gettext('El Salvador') },
            { value: 'GQ', text: gettext('Equatorial Guinea') },
            { value: 'ER', text: gettext('Eritrea') },
            { value: 'EE', text: gettext('Estonia') },
            { value: 'ET', text: gettext('Ethiopia') },
            { value: 'FK', text: gettext('Falkland Islands') },
            { value: 'FO', text: gettext('Faroe Islands') },
            { value: 'FJ', text: gettext('Fiji') },
            { value: 'FI', text: gettext('Finland') },
            { value: 'FR', text: gettext('France') },
            { value: 'GF', text: gettext('French Guiana') },
            { value: 'PF', text: gettext('French Polynesia') },
            { value: 'TF', text: gettext('French Southern Territories') },
            { value: 'FQ', text: gettext('French Southern and Antarctic Territories') },
            { value: 'GA', text: gettext('Gabon') },
            { value: 'GM', text: gettext('Gambia') },
            { value: 'GE', text: gettext('Georgia') },
            { value: 'DE', text: gettext('Germany') },
            { value: 'GH', text: gettext('Ghana') },
            { value: 'GI', text: gettext('Gibraltar') },
            { value: 'GR', text: gettext('Greece') },
            { value: 'GL', text: gettext('Greenland') },
            { value: 'GD', text: gettext('Grenada') },
            { value: 'GP', text: gettext('Guadeloupe') },
            { value: 'GU', text: gettext('Guam') },
            { value: 'GT', text: gettext('Guatemala') },
            { value: 'GN', text: gettext('Guinea') },
            { value: 'GW', text: gettext('Guinea-Bissau') },
            { value: 'GY', text: gettext('Guyana') },
            { value: 'HT', text: gettext('Haiti') },
            { value: 'HM', text: gettext('Heard Island and McDonald Islands') },
            { value: 'HN', text: gettext('Honduras') },
            { value: 'HK', text: gettext('Hong Kong S.A.R., China') },
            { value: 'HU', text: gettext('Hungary') },
            { value: 'IS', text: gettext('Iceland') },
            { value: 'IN', text: gettext('India') },
            { value: 'ID', text: gettext('Indonesia') },
            { value: 'IR', text: gettext('Iran') },
            { value: 'IQ', text: gettext('Iraq') },
            { value: 'IE', text: gettext('Ireland') },
            { value: 'IL', text: gettext('Israel') },
            { value: 'IT', text: gettext('Italy') },
            { value: 'CI', text: gettext('Ivory Coast') },
            { value: 'JM', text: gettext('Jamaica') },
            { value: 'JP', text: gettext('Japan') },
            { value: 'JT', text: gettext('Johnston Island') },
            { value: 'JO', text: gettext('Jordan') },
            { value: 'KZ', text: gettext('Kazakhstan') },
            { value: 'KE', text: gettext('Kenya') },
            { value: 'KI', text: gettext('Kiribati') },
            { value: 'KW', text: gettext('Kuwait') },
            { value: 'KG', text: gettext('Kyrgyzstan') },
            { value: 'LA', text: gettext('Laos') },
            { value: 'LV', text: gettext('Latvia') },
            { value: 'LB', text: gettext('Lebanon') },
            { value: 'LS', text: gettext('Lesotho') },
            { value: 'LR', text: gettext('Liberia') },
            { value: 'LY', text: gettext('Libya') },
            { value: 'LI', text: gettext('Liechtenstein') },
            { value: 'LT', text: gettext('Lithuania') },
            { value: 'LU', text: gettext('Luxembourg') },
            { value: 'MO', text: gettext('Macao S.A.R., China') },
            { value: 'MK', text: gettext('Macedonia') },
            { value: 'MG', text: gettext('Madagascar') },
            { value: 'MW', text: gettext('Malawi') },
            { value: 'MY', text: gettext('Malaysia') },
            { value: 'MV', text: gettext('Maldives') },
            { value: 'ML', text: gettext('Mali') },
            { value: 'MT', text: gettext('Malta') },
            { value: 'MH', text: gettext('Marshall Islands') },
            { value: 'MQ', text: gettext('Martinique') },
            { value: 'MR', text: gettext('Mauritania') },
            { value: 'MU', text: gettext('Mauritius') },
            { value: 'YT', text: gettext('Mayotte') },
            { value: 'FX', text: gettext('Metropolitan France') },
            { value: 'MX', text: gettext('Mexico') },
            { value: 'FM', text: gettext('Micronesia') },
            { value: 'MI', text: gettext('Midway Islands') },
            { value: 'MD', text: gettext('Moldova') },
            { value: 'MC', text: gettext('Monaco') },
            { value: 'MN', text: gettext('Mongolia') },
            { value: 'MS', text: gettext('Montserrat') },
            { value: 'MA', text: gettext('Morocco') },
            { value: 'MZ', text: gettext('Mozambique') },
            { value: 'MM', text: gettext('Myanmar') },
            { value: 'NA', text: gettext('Namibia') },
            { value: 'NR', text: gettext('Nauru') },
            { value: 'NP', text: gettext('Nepal') },
            { value: 'NL', text: gettext('Netherlands') },
            { value: 'AN', text: gettext('Netherlands Antilles') },
            { value: 'NT', text: gettext('Neutral Zone') },
            { value: 'NC', text: gettext('New Caledonia') },
            { value: 'NZ', text: gettext('New Zealand') },
            { value: 'NI', text: gettext('Nicaragua') },
            { value: 'NE', text: gettext('Niger') },
            { value: 'NG', text: gettext('Nigeria') },
            { value: 'NU', text: gettext('Niue') },
            { value: 'NF', text: gettext('Norfolk Island') },
            { value: 'KP', text: gettext('North Korea') },
            { value: 'VD', text: gettext('North Vietnam') },
            { value: 'MP', text: gettext('Northern Mariana Islands') },
            { value: 'NO', text: gettext('Norway') },
            { value: 'OM', text: gettext('Oman') },
            { value: 'QO', text: gettext('Outlying Oceania') },
            { value: 'PC', text: gettext('Pacific Islands Trust Territory') },
            { value: 'PK', text: gettext('Pakistan') },
            { value: 'PW', text: gettext('Palau') },
            { value: 'PS', text: gettext('Palestinian Territory') },
            { value: 'PA', text: gettext('Panama') },
            { value: 'PZ', text: gettext('Panama Canal Zone') },
            { value: 'PG', text: gettext('Papua New Guinea') },
            { value: 'PY', text: gettext('Paraguay') },
            { value: 'YD', text: gettext("People's Democratic Republic of Yemen") },
            { value: 'PE', text: gettext('Peru') },
            { value: 'PH', text: gettext('Philippines') },
            { value: 'PN', text: gettext('Pitcairn') },
            { value: 'PL', text: gettext('Poland') },
            { value: 'PT', text: gettext('Portugal') },
            { value: 'PR', text: gettext('Puerto Rico') },
            { value: 'QA', text: gettext('Qatar') },
            { value: 'RE', text: gettext('Reunion') },
            { value: 'RO', text: gettext('Romania') },
            { value: 'RU', text: gettext('Russia') },
            { value: 'RW', text: gettext('Rwanda') },
            { value: 'SH', text: gettext('Saint Helena') },
            { value: 'KN', text: gettext('Saint Kitts and Nevis') },
            { value: 'LC', text: gettext('Saint Lucia') },
            { value: 'PM', text: gettext('Saint Pierre and Miquelon') },
            { value: 'VC', text: gettext('Saint Vincent and the Grenadines') },
            { value: 'WS', text: gettext('Samoa') },
            { value: 'SM', text: gettext('San Marino') },
            { value: 'ST', text: gettext('Sao Tome and Principe') },
            { value: 'SA', text: gettext('Saudi Arabia') },
            { value: 'SN', text: gettext('Senegal') },
            { value: 'CS', text: gettext('Serbia And Montenegro') },
            { value: 'SC', text: gettext('Seychelles') },
            { value: 'SL', text: gettext('Sierra Leone') },
            { value: 'SG', text: gettext('Singapore') },
            { value: 'SK', text: gettext('Slovakia') },
            { value: 'SI', text: gettext('Slovenia') },
            { value: 'SB', text: gettext('Solomon Islands') },
            { value: 'SO', text: gettext('Somalia') },
            { value: 'ZA', text: gettext('South Africa') },
            { value: 'GS', text: gettext('South Georgia and the South Sandwich Islands') },
            { value: 'KR', text: gettext('South Korea') },
            { value: 'ES', text: gettext('Spain') },
            { value: 'LK', text: gettext('Sri Lanka') },
            { value: 'SD', text: gettext('Sudan') },
            { value: 'SR', text: gettext('Suriname') },
            { value: 'SJ', text: gettext('Svalbard and Jan Mayen') },
            { value: 'SZ', text: gettext('Swaziland') },
            { value: 'SE', text: gettext('Sweden') },
            { value: 'CH', text: gettext('Switzerland') },
            { value: 'SY', text: gettext('Syria') },
            { value: 'TW', text: gettext('Taiwan') },
            { value: 'TJ', text: gettext('Tajikistan') },
            { value: 'TZ', text: gettext('Tanzania') },
            { value: 'TH', text: gettext('Thailand') },
            { value: 'TG', text: gettext('Togo') },
            { value: 'TK', text: gettext('Tokelau') },
            { value: 'TO', text: gettext('Tonga') },
            { value: 'TT', text: gettext('Trinidad and Tobago') },
            { value: 'TN', text: gettext('Tunisia') },
            { value: 'TR', text: gettext('Turkey') },
            { value: 'TM', text: gettext('Turkmenistan') },
            { value: 'TC', text: gettext('Turks and Caicos Islands') },
            { value: 'TV', text: gettext('Tuvalu') },
            { value: 'PU', text: gettext('U.S. Miscellaneous Pacific Islands') },
            { value: 'VI', text: gettext('U.S. Virgin Islands') },
            { value: 'UG', text: gettext('Uganda') },
            { value: 'UA', text: gettext('Ukraine') },
            { value: 'SU', text: gettext('Union of Soviet Socialist Republics') },
            { value: 'AE', text: gettext('United Arab Emirates') },
            { value: 'GB', text: gettext('United Kingdom') },
            { value: 'US', text: gettext('United States') },
            { value: 'UM', text: gettext('United States Minor Outlying Islands') },
            { value: 'UY', text: gettext('Uruguay') },
            { value: 'UZ', text: gettext('Uzbekistan') },
            { value: 'VU', text: gettext('Vanuatu') },
            { value: 'VA', text: gettext('Vatican') },
            { value: 'VE', text: gettext('Venezuela') },
            { value: 'VN', text: gettext('Vietnam') },
            { value: 'WK', text: gettext('Wake Island') },
            { value: 'WF', text: gettext('Wallis and Futuna') },
            { value: 'EH', text: gettext('Western Sahara') },
            { value: 'YE', text: gettext('Yemen') },
            { value: 'ZM', text: gettext('Zambia') },
            { value: 'ZW', text: gettext('Zimbabwe') }
          ],
          sorters: [
            {
              dir: 'asc',
              prop: 'label'
            }
          ]
        },
        validators: {
          required: true
        }
      },
      {
        type: 'textInput',
        name: 'email',
        value: '',
        label: gettext('Email'),
        validators: {
          patternType: 'email'
        }
      },
      {
        type: 'tagInput',
        name: 'comment',
        value: '',
        label: gettext('Tags'),
        hint: gettext(
          'This field is automatically filled with the subject of the certificate if left blank.'
        )
      }
    ],
    buttons: [
      {
        template: 'submit',
        text: gettext('Create'),
        execute: {
          type: 'url',
          url: '/system/certificate/ssl'
        }
      },
      {
        template: 'cancel',
        execute: {
          type: 'url',
          url: '/system/certificate/ssl'
        }
      }
    ]
  };
}
