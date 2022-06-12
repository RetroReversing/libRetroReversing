module.exports = {
  "root": true,
  "parser": "@typescript-eslint/parser",
  "extends": [
    "eslint:recommended"
  ],
  "plugins": [
    "@typescript-eslint",
    "@babel",
    "react",
    "react-hooks"
  ],
  "env": {
    "browser": true,
    "commonjs": true,
    "es6": true,
    "jest": true,
    "node": true,
    "worker": true
  },
  "parserOptions": {
    "ecmaVersion": 6,
    "sourceType": "module",
    "ecmaFeatures": {
      "jsx": true,
      "generators": true,
      "experimentalObjectRestSpread": true
    }
  },
  "settings": {
    "react": {
      "version": "16.3.0"
    }
  },
  "overrides": [
    {
      "files": ["*.tsx?", "*.jsx", "*.js"],
      "extends": ["plugin:@typescript-eslint/recommended"],
      "rules": {
        "@typescript-eslint/explicit-module-boundary-types": "off",
        "@typescript-eslint/no-empty-function": "off",
        "@typescript-eslint/ban-types": "off",
        "@typescript-eslint/no-var-requires": "off",
        "@typescript-eslint/ban-ts-comment": "off",
        "@typescript-eslint/no-explicit-any": "off",
        "@typescript-eslint/no-non-null-assertion": "off",
      },
    },
  ],
  "rules": {
    "array-callback-return": "error",
    "arrow-spacing": "error",
    "camelcase": "error",
    "comma-spacing": "off",
    "@typescript-eslint/comma-spacing": ["error"],
    "curly": "error",
    "default-case": [
      "error",
      {
        "commentPattern": "^no default$"
      }
    ],
    "dot-location": [
      "error",
      "property"
    ],
    "eol-last": "error",
    "eqeqeq": [
      "error",
      "smart"
    ],
    "indent": [
      "error",
      2,
      {
        "SwitchCase": 1
      }
    ],
    "guard-for-in": "error",
    "keyword-spacing": "error",
    "key-spacing": "error",
    "new-parens": "error",
    "no-array-constructor": "error",
    "no-caller": "error",
    "no-cond-assign": [
      "error",
      "always"
    ],
    "no-const-assign": "error",
    "no-console": "off",
    "no-control-regex": "error",
    "no-delete-var": "error",
    "no-dupe-args": "error",
    "no-dupe-class-members": "error",
    "no-dupe-keys": "error",
    "no-duplicate-case": "error",
    "no-empty-character-class": "error",
    "no-empty-pattern": "error",
    "no-eval": "error",
    "no-ex-assign": "error",
    "no-extend-native": "error",
    "no-extra-bind": "error",
    "no-extra-label": "error",
    "no-fallthrough": "error",
    "no-func-assign": "error",
    "no-global-assign": "error",
    "no-implied-eval": "error",
    "no-invalid-regexp": "error",
    "no-iterator": "error",
    "no-label-var": "error",
    "no-labels": [
      "error",
      {
        "allowLoop": false,
        "allowSwitch": false
      }
    ],
    "no-lone-blocks": "error",
    "no-loop-func": "error",
    "no-mixed-operators": [
      "error",
      {
        "groups": [
          [
            "&",
            "|",
            "^",
            "~",
            "<<",
            ">>",
            ">>>"
          ],
          [
            "==",
            "!=",
            "===",
            "!==",
            ">",
            ">=",
            "<",
            "<="
          ],
          [
            "&&",
            "||"
          ],
          [
            "in",
            "instanceof"
          ]
        ],
        "allowSamePrecedence": false
      }
    ],
    "no-multi-str": "error",
    "no-new-func": "error",
    "no-new-object": "error",
    "no-new-symbol": "error",
    "no-new-wrappers": "error",
    "no-obj-calls": "error",
    "no-octal": "error",
    "no-octal-escape": "error",
    "no-redeclare": "error",
    "no-regex-spaces": "error",
    "no-restricted-syntax": [
      "error",
      "LabeledStatement",
      "WithStatement"
    ],
    "no-script-url": "error",
    "no-self-assign": "error",
    "no-self-compare": "error",
    "no-sequences": "error",
    "no-shadow-restricted-names": "error",
    "no-sparse-arrays": "error",
    "no-template-curly-in-string": "error",
    "no-this-before-super": "error",
    "no-throw-literal": "error",
    "no-trailing-spaces": "error",
    "no-undef": "off", // typescript handles this instead: https://github.com/eslint/typescript-eslint-parser/issues/437
    "no-unexpected-multiline": "error",
    "no-unreachable": "error",
    "no-unsafe-negation": "error",
    "no-unused-expressions": "error",
    "no-unused-labels": "error",
    "no-unused-vars": "off",
    "@typescript-eslint/no-unused-vars": [
      "error",
      {
        "vars": "local",
        "args": "none"
      }
    ],

    // note you must disable the base rule as it can report incorrect errors
    "no-use-before-define": "off",
    "@typescript-eslint/no-use-before-define": [
      "error",
      "nofunc"
    ],
    "no-useless-computed-key": "error",
    "no-useless-concat": "error",
    "no-useless-constructor": "error",
    "no-useless-escape": "error",
    "no-useless-rename": [
      "error",
      {
        "ignoreDestructuring": false,
        "ignoreImport": false,
        "ignoreExport": false
      }
    ],
    "no-with": "error",
    "no-whitespace-before-property": "error",
    // Base rule must be disabled for the @typescript version to work.
    "object-curly-spacing": "off",
    "@typescript-eslint/object-curly-spacing": ["error", "always"],
    "operator-assignment": [
      "error",
      "always"
    ],
    "radix": "error",
    "require-yield": "error",
    "rest-spread-spacing": [
      "error",
      "never"
    ],
    "semi": "error",
    "strict": [
      "error",
      "never"
    ],
    "space-infix-ops": [
      "error",
      {
        "int32Hint": false
      }
    ],
    "unicode-bom": [
      "error",
      "never"
    ],
    "use-isnan": "error",
    "valid-typeof": "error",
    "react/jsx-boolean-value": "error",
    "react/jsx-closing-bracket-location": "off",
    "react/jsx-curly-spacing": "error",
    "react/jsx-equals-spacing": [
      "error",
      "never"
    ],
    "react/jsx-first-prop-new-line": [
      "error",
      "multiline"
    ],
    "react/jsx-handler-names": "off",
    "react/jsx-indent": [
      "error",
      2
    ],
    "react/jsx-indent-props": [
      "error",
      2
    ],
    "react/jsx-key": "error",
    "react/jsx-max-props-per-line": [
      "error",
      {
        "maximum": 3
      }
    ],
    "react/jsx-no-comment-textnodes": "error",
    "react/jsx-no-duplicate-props": [
      "error",
      {
        "ignoreCase": true
      }
    ],
    "react/jsx-no-undef": "error",
    "react/jsx-pascal-case": [
      "error",
      {
        "allowAllCaps": true,
        "ignore": []
      }
    ],
    "react/jsx-sort-props": "error",
    "react/jsx-tag-spacing": "error",
    "react/jsx-uses-react": "error",
    "react/jsx-uses-vars": "error",
    "react/jsx-wrap-multilines": "error",
    "react/no-deprecated": "error",
    "react/no-did-mount-set-state": "error",
    "react/no-did-update-set-state": "error",
    "react/no-direct-mutation-state": "error",
    "react/no-is-mounted": "error",
    "react/no-unused-prop-types": "error",
    "react/prefer-es6-class": "error",
    "react/prefer-stateless-function": "error",
    "react/prop-types": "error",
    "react/react-in-jsx-scope": "error",
    "react/require-render-return": "error",
    "react/self-closing-comp": "error",
    "react/sort-comp": "error",
    "react/sort-prop-types": "error",
    "react/style-prop-object": "error",
    "react/void-dom-elements-no-children": "error",
    "react-hooks/rules-of-hooks": "error",
    "react-hooks/exhaustive-deps": "off",
    "testing-library/prefer-explicit-assert": "error",
    "testing-library/prefer-wait-for": "error",
    "@typescript-eslint/member-delimiter-style": [
      "error",
      {
        "multiline": {
          "delimiter": "semi",
          "requireLast": true
        },
        "singleline": {
          "delimiter": "comma",
          "requireLast": false
        }
      }
    ],
    "@typescript-eslint/type-annotation-spacing": "error"
  }
};
