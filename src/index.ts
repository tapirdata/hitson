import bindings = require('bindings');
import { AddonFactory } from './types';

const addonFactory = bindings('wson_addon') as AddonFactory;
export default addonFactory;
