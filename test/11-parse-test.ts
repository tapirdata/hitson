/* eslint-disable @typescript-eslint/no-unsafe-assignment */
/* eslint-disable @typescript-eslint/restrict-template-expressions */
/* eslint-disable @typescript-eslint/no-unsafe-member-access */
/* eslint-disable @typescript-eslint/no-unsafe-call */
import _ = require('lodash');
import { expect } from 'chai';

import { saveRepr } from './fixtures/helpers';
import setups from './fixtures/setups';
import pairs from './fixtures/stringify-pairs';
import wsonFactory, { ParseError } from './wsonFactory';

for (const setup of setups) {
  describe(setup.name, () => {
    const wson = wsonFactory(setup.options);
    describe('parse', () => {
      for (const pair of pairs) {
        if (!_.has(pair, 's')) {
          continue;
        }
        if (pair.parseFailPos != null) {
          it(`should fail to parse '${pair.s}' at ${pair.parseFailPos}`, () => {
            let e;
            try {
              wson.parse(pair.s, { backrefCb: pair.backrefCb });
            } catch (someE) {
              e = someE as ParseError;
            }
            if (e == null) {
              throw new Error('ParseError expected');
            }
            expect(e.name).to.be.equal('ParseError');
            expect(e.pos).to.be.equal(pair.parseFailPos);
          });
        } else {
          it(`should parse '${pair.s}' as ${saveRepr(pair.x)}`, () => {
            expect(wson.parse(pair.s, { backrefCb: pair.backrefCb })).to.be.deep.equal(pair.x);
          });
        }
      }
    });
  });
}
