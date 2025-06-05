const fs = require('fs');
const path = require('path');
const { default: convert } = require('@openapi-contrib/openapi-schema-to-json-schema');

// Usage: node fix-schema.js input.json output.json

const inputPath = process.argv[2];
const outputPath = process.argv[3];

if (!inputPath || !outputPath) {
  console.error('Usage: node fix-schema.js input.json output.json');
  process.exit(1);
}

const input = JSON.parse(fs.readFileSync(inputPath, 'utf8'));
const output = convert(input);
fs.writeFileSync(outputPath, JSON.stringify(output, null, 2));
