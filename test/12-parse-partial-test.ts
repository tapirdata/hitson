import { expect } from 'chai';

import wsonFactory from './wsonFactory';
import setups from './fixtures/setups';
import pairs from './fixtures/partial-pairs';
import { saveRepr } from './fixtures/helpers'

for (const setup of setups) {
  describe(setup.name, () => {
    const wson = wsonFactory(setup.options);

    function collectPartial(s, nrs, backrefCb) {
      let result: any[] = [];
      let nrIdx = 0;

      function cb (isValue, value, pos) {
        result.push(isValue);
        result.push(value);
        result.push(pos);
        return nrs[nrIdx++];
      };

      wson.parsePartial(s, {howNext: nrs[nrIdx++], cb, backrefCb});

      return result;
    };

    describe('parse partial', () => {
      for (const pair of pairs) {
        if (pair.failPos != null) {
          it(`should fail to parse '${pair.s}' at ${pair.failPos}`, function() {
            let e
            try {
              collectPartial(pair.s, pair.nrs, pair.backrefCb);
            } catch (e_) {
              e = e_;
            }
            expect(e.name).to.be.equal('ParseError');
            expect(e.pos).to.be.equal(pair.failPos);
          }
          );
        } else {
          it(`should parse '${pair.s}' as ${saveRepr(pair.col)} (nrs=${saveRepr(pair.nrs)})`,
            () => expect(collectPartial(pair.s, pair.nrs, pair.backrefCb)).to.be.deep.equal(pair.col)
          );
        }
      }
    });
  });
}



