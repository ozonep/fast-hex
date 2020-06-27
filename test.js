const pkg = require(".");

console.log("Encoding OK");

const dest1 = new Uint8Array(a.length / 2);

pkg.decodeHexVec(dest1, a);
console.log("Decoding OK");
