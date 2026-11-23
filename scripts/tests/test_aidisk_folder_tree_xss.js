"use strict";

const assert = require("assert");
const fs = require("fs");
const vm = require("vm");

const sourcePath = "trunk/user/www/n56u_ribbon_fixed/disk_folder_tree.js";
const source = fs.readFileSync(sourcePath, "utf8");
const context = {};
vm.runInNewContext(source, context, { filename: sourcePath });

const payload = '"><img src=x onerror=alert(1)>';
assert.strictEqual(
  context.escapeFolderTreeHtml(payload),
  "&quot;&gt;&lt;img src=x onerror=alert(1)&gt;"
);
assert.strictEqual(
  context.escapeFolderTreeHtml("'&<>\""),
  "&#39;&amp;&lt;&gt;&quot;"
);
assert.strictEqual(
  context.showFolderTreeText("A  &nbsp; <B>"),
  "A&nbsp;&nbsp;&amp;nbsp;&nbsp;&lt;B&gt;"
);
assert.strictEqual(context.escapeFolderTreeHtml(null), "");

const buildTreeStart = source.indexOf("function BuildTree()");
assert.notStrictEqual(buildTreeStart, -1);
const buildTree = source.slice(buildTreeStart);
assert.ok(buildTree.includes("shown_ItemText = showFolderTreeText(short_ItemText);"));
assert.ok(buildTree.includes("shown_ItemText = showFolderTreeText(ItemText);"));
assert.ok(buildTree.includes("title=\"'+escapeFolderTreeHtml(ItemText)+'\">"));

console.log("AiDisk folder-tree HTML escaping regression: PASS");
