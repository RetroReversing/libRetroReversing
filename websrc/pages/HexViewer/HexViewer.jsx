import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import css from "./style.css";
import { FixedSizeList as List } from 'react-window';
// var React = require('react');
var AsciiTable = require('./AsciiViewer');
var AsciiSet = AsciiTable.AsciiSet;
var PixelSet = require('./PixelTable').PixelSet;
var PixelSet2 = require('./PixelTable').PixelSet2;
var createReactClass = require('create-react-class');



var Item = createReactClass({
	activate: function () {
		this.props.activate(this.props.index);
	},
	clear: function() {
		this.props.clear();
	},
	render: function() {
		var classes = (this.props.active ? 'active' : '') + (this.props.value == -1 ? ' none' : '');
		return (<li className={classes} onMouseOver={this.activate} onMouseLeave={this.clear}>{this.props.byteString}</li>);
	}
});

var Set = createReactClass({
	activate: function () {
		this.props.activateSet(this.props.index);
	},
	clear: function() {
		this.props.clearSet();
	},
	render: function() {
		var items = this.props.set.map(function(b, i) {
			var byteString = "";

			if (b != -1 ) {
				byteString = b.toString(16);

				if(byteString.length == 1) {
					byteString = "0" + byteString;
				}
			}

			var active = this.props.activeItem == i && this.props.active;
			return (<Item index={i} value={b} byteString={byteString} active={active} activate={this.props.activateItem} clear={this.props.clearItem}/>);
		}.bind(this));

		return (
			<ul className={"setHex" + (this.props.active ? ' active' : '')} onMouseOver={this.activate} onMouseLeave={this.clear}>
				{items}
			</ul>
		);
	}
});

// 
// # For Highlighting - Each row has both an active set and an active item
// The active set is one of the 4 sets of 4 bytes and the active itsem is which of those 4 bytes was selected.
// 
var Row = createReactClass({
	getInitialState: function() {
		return ({
			activeSet: -1,
			activeItem: -1
		});
	},
	setActiveSet: function (activeSet) {
		if(this.props.sets[activeSet][this.state.activeItem] == -1) return;
		this.setState({activeSet: activeSet});
	},
	clearActiveSet: function () {
		this.setState({activeSet: -1});
	},
	setActiveItem: function (activeItem) {
		this.setState({activeItem: activeItem});
	},
	clearActiveItem: function () {
		this.setState({activeItem: -1});
	},
	activate: function () {
		this.setActiveSet(this.props.index);
	},
	render: function() {
		var sets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,

				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem
			};

			return (<Set {...props}/>);
		}.bind(this));


		var asciiSets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				setIndex: i,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,
				activeSet: this.state.activeSet,
				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem
			};

			return (<AsciiSet {...props}/>);
		}.bind(this));

		var pixelSets = this.props.sets.map(function(set, i) {
			var active = this.state.activeSet == i ? true : false;

			var props = {
				set: set,
				setIndex: i,
				index: i,
				active: active,
				activeItem:   this.state.activeItem,
				activeSet: this.state.activeSet,
				activateSet:  this.setActiveSet,
				clearSet:     this.clearActiveSet,
				activateItem: this.setActiveItem,
				clearItem:    this.clearActiveItem
			};

			return (<PixelSet {...props}/>);
		}.bind(this));

		return (
			<div className="row">
				<div className="heading">{this.props.heading}:</div>
        <div className="hex-sets">
          {sets}
        </div>
				<div className="ascii">
					{asciiSets}
				</div>
				<div className="pixels">
					{pixelSets}
				</div>
			</div>
		);
	}
});

function Hex(props) {
  const pad = "000000";


		const createRow = ({ index, style }) => {
			const row = props.rows[index];
			var heading = ''+index*props.bytesper;
				heading = pad.substring(0, pad.length - heading.length) + heading;
			return <div style={style}><Row sets={row} heading={heading}/></div>
		};

		return (
			<div className="hexviewer">
				<div className="hex">
				<List
					height={350}
					itemCount={props.rows.length}
					itemSize={25}
					width={'100%'}
				>
					{createRow}
				</List>
				</div>
			</div>
		);
}

const HexViewer = function(props) {
		var rowChunk = props.rowLength, setChunk = props.setLength;
		var rows = [], row = [], set = [], sets = [];
		
		var buffer = [];
		var bytes = props.buffer.length;

		if(Buffer.isBuffer(props.buffer)) {
			for (var ii = 0; ii < bytes; ii++) {
				buffer.push(props.buffer[ii]);
			}
		} else {
			buffer = props.buffer;
		}

		for (var i = 0; i<bytes; i+=rowChunk) {
			sets = [];
			let temparray = buffer.slice(i,i+rowChunk);

			for(var z = temparray.length; z < rowChunk; z++) {
				temparray.push(-1);
			}
			row = [];
			for (let x=0,k=temparray.length; x<k; x+=setChunk) {
				set = temparray.slice(x,x+setChunk);

				for(z = set.length; z < setChunk; z++) {
					set.push(-1);
				}
				row.push(set);

			}
			rows.push(row);
		}

		return (
			<Hex rows={rows} bytesper={rowChunk} />
		);
	};

export default HexViewer;
