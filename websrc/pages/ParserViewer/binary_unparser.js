import { noop } from 'lodash';
import React from 'react';
import ReactDOM from "react-dom";
var Context = require("./context").Context;
var Parser = require("./binary_parser");
var vm = require("vm");

var aliasRegistry = {};
var FUNCTION_PREFIX = "___parser_";

class UnParser {
  constructor(obj) {
    this.options = {};
    this.constructorFn = noop;
    obj && Object.assign(this, obj);

    if (obj) {
      // this.prototype = Object.getPrototypeOf(obj);
      this.parser = obj;
      this.resolveReferences = obj.resolveReferences;
    }
    this.generateString = this.generateStringUnparser;
  }

  convertToUnParser(parser) {
    return new UnParser(parser);
  }

  unparse(data, buffer, react={}) {
    if (!this.unparserCompiled) {
      this.compileUnparser();
    }

    return this.unparserCompiled(buffer, this.constructorFn, data, react);
  }

  compileUnparser() {
    var src = `(function unparser(buffer, constructorFn, vars) {
      const React = window.React;
      console.error("compileUnparser react:", React, window.React, window);
      var extraClasses="";` + this.getUnparserCode() + " })";
    this.unparserCompiled = vm.runInThisContext(src);
  };

  getUnparserCode() {
    var ctx = new Context();
    ctx.isUnparser = true;

    if (!this.alias) {
      this.addRawUnparserCode(ctx);
    } else {
      console.error('TODO: Aliased code not supported');
      this.addAliasedCode(ctx);
    }

    if (this.alias) {
      ctx.pushCode("return {0}(0).result;", FUNCTION_PREFIX + this.alias);
    } else {
      ctx.pushCode("return buffer;");
    }

    return ctx.code;
  };

  addRawUnparserCode(ctx) {
    ctx.pushCode("var offset = 0;");

    this.generateUnparser(ctx);

    this.resolveReferences(ctx);

    ctx.pushCode("return buffer;");
  };

  generateFormatter (ctx, varName, formatter) {
    if (typeof formatter === "function") {
      // ctx.pushCode("{0} = ({1}).call(this, {0});", varName, formatter);
      ctx.pushCode("console.error('generate formatter: {0} = ({1}).call(this, {0});');", varName, formatter);
    }
  }

  generateUnparser(ctx) {
    if (this.type) {
      if (this["generate" + this.type]) {
        this["generate" + this.type].bind(this)(ctx);
      } else {
        console.error("No UnParser Generator for", this.type);
      }
    }

    var varName = ctx.generateVariable(this.varName);
    if (this.options.formatter) {
      this.generateFormatter(ctx, varName, this.options.formatter);
    }

    return this.generateNext(ctx);
  };

  generateStringUnparser(ctx) {
    var name = ctx.generateVariable(this.varName);
    var start = ctx.generateTmpVariable();
    this.generateAssert(ctx);

    if (this.options.length && this.options.zeroTerminated) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode(
        "while(buffer.readUInt8(offset++) !== 0 && offset - {0}  < {1});",
        start,
        this.options.length
      );
      ctx.pushCode(
        "buffer.write('{0}', {1}, offset - {2} < {3} ? offset - 1 : offset, '{3}');",
        name,
        start,
        this.options.length,
        this.options.encoding
      );
    } else if (this.options.length) {
      ctx.pushCode(
        "buffer.write({0}, offset, offset + {2}, '{3}');",
        name,
        start,
        ctx.generateOption(this.options.length),
        this.options.encoding
      );
      ctx.pushCode("offset += {0};", ctx.generateOption(this.options.length));
    } else if (this.options.zeroTerminated) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode("while(buffer.readUInt8(offset++) !== 0);");
      ctx.pushCode(
        "{0} = buffer.toString('{1}', {2}, offset - 1);",
        name,
        this.options.encoding,
        start
      );
    } else if (this.options.greedy) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode("while(buffer.length > offset++);");
      ctx.pushCode(
        "{0} = buffer.toString('{1}', {2}, offset);",
        name,
        this.options.encoding,
        start
      );
    }
    if (this.options.stripNull) {
      ctx.pushCode("{0} = {0}.replace(/\\x00+$/g, '')", name);
    }
  };

  // Recursively call code generators and append results
  generateNext(ctx) {
    if (this.next) {
      ctx = this.convertToUnParser(this.next).generate(ctx);
    }

    return ctx;
  };

  generate(ctx) {
    if (this.type && this["generate" + this.type]) {
      this.generateAssert(ctx);
      this["generate" + this.type](ctx);
    } else {
      console.error("No UnParser Generator for", this.type);
    }

    var varName = ctx.generateVariable(this.varName);
    if (this.options.formatter) {
      this.generateFormatter(ctx, varName, this.options.formatter);
    }

    return this.generateNext(ctx);
  };

  generateNest(ctx) {
    var nestVar = ctx.generateVariable(this.varName);

    if (this.options.type instanceof Parser.Parser) {
      if (this.varName && !ctx.isUnparser) {
        ctx.pushCode("{0} = {};", nestVar);
      }
      ctx.pushPath(this.varName);
      this.convertToUnParser(this.options.type).generate(ctx);
      ctx.popPath(this.varName);
    } else if (aliasRegistry[this.options.type]) {
      var tempVar = ctx.generateTmpVariable();
      ctx.pushCode(
        "var {0} = {1}(offset);",
        tempVar,
        FUNCTION_PREFIX + this.options.type
      );
      ctx.pushCode("{0} = {1}.result; offset = {1}.offset;", nestVar, tempVar);
      if (this.options.type !== this.alias) ctx.addReference(this.options.type);
    }
  };

  generateAssert (ctx) {
    if (!this.options.assertWithoutErroring) {
      ctx.pushCode(`extraClasses="";`);
      return;
    }
  }

  generateArray(ctx) {
    var type = this.options.type;
    var counter = ctx.generateTmpVariable();
    var arrayName = ctx.generateVariable(this.varName);
    var item = ctx.generateTmpVariable();
    ctx.pushCode("for (var {0} = 0; {0} < {1}.length; {0}++) {", counter, arrayName);

    if (type instanceof Parser.Parser) {
      if (ctx.isUnparser) {
        ctx.pushCode("{0} = {1}[{2}];", item, arrayName, counter);
      } else {
        ctx.pushCode("var {0} = {};", item);
      }

      ctx.pushScope(item);
      this.convertToUnParser(type).generate(ctx);
      ctx.popScope();
    }

    ctx.pushCode("}");
  };

  generateChoice(ctx) {
    var tag = ctx.generateOption(this.options.tag);
    // if (this.varName) {
    //   ctx.pushCode("{0} = {};", ctx.generateVariable(this.varName));
    // }
    ctx.pushCode("switch({0}) {", tag);
    Object.keys(this.options.choices).forEach(function (tag) {
      var type = this.options.choices[tag];

      ctx.pushCode("case {0}:", tag);
      this.generateChoiceCase(ctx, this.varName, type);
      ctx.pushCode("break;");
    }, this);
    ctx.pushCode("default:");
    if (this.options.defaultChoice) {
      this.generateChoiceCase(ctx, this.varName, this.options.defaultChoice);
    } else {
      ctx.generateError('"Met undefined tag value " + {0} + " at choice"', tag);
    }
    ctx.pushCode("}");
  };

  generateChoiceCase(ctx, varName, type) {
    if (typeof type === "string") {
      if (!aliasRegistry[type]) {
        // ctx.pushCode(
        //   "{0} = buffer.read{1}(offset);",
        //   ctx.generateVariable(this.varName),
        //   NAME_MAP[type]
        // );
        // ctx.pushCode("offset += {0};", PRIMITIVE_TYPES[NAME_MAP[type]]);
      } else {
        var tempVar = ctx.generateTmpVariable();
        ctx.pushCode("var {0} = {1}(offset);", tempVar, FUNCTION_PREFIX + type);
        ctx.pushCode(
          "{0} = {1}.result; offset = {1}.offset;",
          ctx.generateVariable(this.varName),
          tempVar
        );
        if (type !== this.alias) ctx.addReference(type);
      }
    } else if (type instanceof Parser.Parser) {
      ctx.pushPath(varName);
      this.convertToUnParser(type).generate(ctx);
      ctx.popPath(varName);
    }
  };
}

var PRIMITIVE_TYPES = {
  UInt8: 1,
  UInt16LE: 2,
  UInt16BE: 2,
  UInt32LE: 4,
  UInt32BE: 4,
  Int8: 1,
  Int16LE: 2,
  Int16BE: 2,
  Int32LE: 4,
  Int32BE: 4,
  FloatLE: 4,
  FloatBE: 4,
  DoubleLE: 8,
  DoubleBE: 8
};

Object.keys(PRIMITIVE_TYPES).forEach(function(type) {
  UnParser.prototype["generate" + type] = function(ctx) {
    ctx.pushCode(
      "buffer.write{1}({0}, offset);",
      ctx.generateVariable(this.varName),
      type
    );
    ctx.pushCode("offset += {0};", PRIMITIVE_TYPES[type]);
  };
});

export class FileUnparser extends UnParser { };

export class ReactUnparser extends UnParser {

  constructor(obj, reactElement='\"div\"') {
    super(obj);
    obj && Object.assign(this, obj);

    if (obj) {
      // this.prototype = Object.getPrototypeOf(obj);
      this.parser = obj;
      this.resolveReferences = obj.resolveReferences;
    }
    this.generateString = this.generateStringUnparser;
    this.reactElement = reactElement;
  }

  convertToUnParser(parser) {
    return new ReactUnparser(parser);
  }

  generateStringUnparser(ctx) {
    var name = ctx.generateVariable(this.varName);
    var start = ctx.generateTmpVariable();

    if (this.options.length && this.options.zeroTerminated) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode(
        "while(buffer.readUInt8(offset++) !== 0 && offset - {0}  < {1});",
        start,
        this.options.length
      );
      ctx.pushCode(
        "buffer.write('{0}', {1}, offset - {2} < {3} ? offset - 1 : offset, '{3}');",
        name,
        start,
        this.options.length,
        this.options.encoding
      );
    } else if (this.options.length) {
      const classNames = name.split('.');
      const classNameStr = classNames.slice(1).join(' ');
      ctx.pushCode(`{ let extraClasses="";`);

      if (this.options.shouldBe) {
        ctx.pushCode(`if (${name} !== \`${this.options.shouldBe}\`) {
          extraClasses+=" invalid";
          console.error("Parser Warning ${name} was value:",${name}, " but should have been:", ${this.options.shouldBe});
        }`);
      }

      ctx.pushCode(
        `
        const reactElement = React.createElement(
          ${this.reactElement},
          { className: "${classNameStr}"+extraClasses,
          options:${JSON.stringify(this.options)},
          keys: "${classNames}".replace('',),
          title: "${classNameStr}", 
          key:"${name}" + offset,
          offset: offset,
          uniqueKey:"${name}" + offset, 
          value: {0}.substring(0,{2}) },
          {0}.replace(/ /g,'_').substring(0,{2})
        );
        buffer.push(reactElement);`,
        name,
        start,
        ctx.generateOption(this.options.length),
        this.options.encoding
      );
      ctx.pushCode("}\n offset += {0};", ctx.generateOption(this.options.length));
    } else if (this.options.zeroTerminated) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode("while(buffer.readUInt8(offset++) !== 0);");
      ctx.pushCode(
        "{0} = buffer.toString('{1}', {2}, offset - 1);",
        name,
        this.options.encoding,
        start
      );
    } else if (this.options.greedy) {
      ctx.pushCode("var {0} = offset;", start);
      ctx.pushCode("while(buffer.length > offset++);");
      ctx.pushCode(
        "{0} = buffer.toString('{1}', {2}, offset);",
        name,
        this.options.encoding,
        start
      );
    }
    if (this.options.stripNull) {
      ctx.pushCode("{0} = {0}.replace(/\\x00+$/g, '')", name);
    }
  };

};

ReactUnparser.prototype.generateAssert = function(ctx) {
  if (!this.options.assertWithoutErroring) {
    ctx.pushCode(`extraClasses="";`);
    return;
  }

  var varName = ctx.generateVariable(this.varName);

  switch (typeof this.options.assertWithoutErroring) {
    case "function":
      ctx.pushCode(
        "if (!({0}).call(vars, {1})) {",
        this.options.assertWithoutErroring,
        varName
      );
      break;
    case "number":
      ctx.pushCode("if ({0} !== {1}) {", this.options.assertWithoutErroring, varName);
      break;
    case "string":
      ctx.pushCode('if ("{0}" !== {1}) {', this.options.assertWithoutErroring, varName);
      break;
    default:
      throw new Error(
        "Assert option supports only strings, numbers and assert functions."
      );
  }
  ctx.pushCode(`console.error("Assert error: ${varName} is ", ${varName});
  extraClasses+="invalid";`);
  ctx.pushCode("}");
};

function writeReactElement(ctx, reactElement, name, options, valueFormatter) {
  const classNames = name.split('.');
  const classNameStr = classNames.slice(1).join(' ');
  const length = ctx.generateOption(options.length);
  const elementOptions = `{
    className: "${classNameStr} "+extraClasses,
    options:${JSON.stringify(options)},
    keys: "${classNames}",
    title: "${classNameStr}",
    key:"${name}" + offset,
    offset: offset,
    uniqueKey:"${name}" + offset,
    value: ${name} // .substring(0,${length})
  }`;
  const code= `
  reactElement = React.createElement(${reactElement}, ${elementOptions}, ${valueFormatter(name)});
  buffer.push(reactElement);
  `;
  ctx.pushCode(
    code,
    name,
    null,
    ctx.generateOption(options.length),
    options.encoding
  );
}

const hexFormatter = (chars, endian="BE", value) => {
  if (endian === "LE") {
    return `
    "0x"+
    (${value}
      .toString(16)
      .replace(/^(.(..)*)$/, "0$1")
      .match(/../g)
      .reverse()
      .join("") 
    )
    `;
  }
  return `"0x"+(${value}+ Math.pow(16, ${chars})).toString(16).slice(-${chars}).toUpperCase()`;
}

Object.keys(PRIMITIVE_TYPES).forEach(function(type) {
  ReactUnparser.prototype["generate" + type] = function(ctx) {
    const name = ctx.generateVariable(this.varName);
    const numberOfCharsInHexString = (PRIMITIVE_TYPES[type])*2;
    const endian = type.slice(-2);
    writeReactElement(ctx, this.reactElement, name, this.options, hexFormatter.bind(null, numberOfCharsInHexString, endian));
    // offset is still used in unparser to distinguish between 2 elements with same name
    ctx.pushCode("offset += {0};", PRIMITIVE_TYPES[type]);
  };
});
