import React from "react";
import PropTypes from "prop-types";
import clsx from "clsx";
import { withStyles } from "@material-ui/core/styles";
import TableCell from "@material-ui/core/TableCell";
import Paper from "@material-ui/core/Paper";
import { AutoSizer, Column, Table } from "react-virtualized";
import { TableSortLabel } from '@material-ui/core';

const headerRenderer = (order,setOrder, orderBy,setOrderBy, headerHeight, columns, { label, columnIndex }) => {
  function updateOrder() {
    const isAsc = orderBy === label && order === 'asc';
    setOrder(isAsc ? 'desc' : 'asc');
    setOrderBy(label);
  }
  return (
    <TableCell
      component="div"
      variant="head"
      style={{ height: headerHeight }}
      align={columns[columnIndex].numeric || false ? "right" : "left"}
    >
    <TableSortLabel
              active={orderBy === label}
              direction={orderBy === label ? order : 'asc'}
              onClick={updateOrder}>
      <span>{label}</span>
    </TableSortLabel>
    </TableCell>
  );
};

const cellRenderer = (columns, rowHeight,{ cellData, columnIndex }) => {
  // console.error("cellRenderer", columns, rowHeight, cellData, columnIndex );
  return (
    <TableCell
      component="div"
      variant="body"
      style={{ height: rowHeight }}
      align={
        (columnIndex != null && columns[columnIndex].numeric) || false
          ? "right"
          : "left"
      }
    >
      {cellData}
    </TableCell>
  );
};

export default function VirtualizedTable({
  columns,
  rowHeight=48,
  headerHeight=56,
  orderBy, 
  setOrderBy,
  order,
  setOrder,
  ...tableProps
}) {

  return (
      <AutoSizer>
        {({ height, width }) => (
          <Table
            height={height}
            width={width}
            rowHeight={rowHeight}
            gridStyle={{
              direction: "inherit"
            }}
            headerHeight={headerHeight}
            // className={classes.table}
            {...tableProps}
            // rowClassName={this.getRowClassName}
            rowStyle = {{
              display: "flex",
              alignItems: "center",
              boxSizing: "border-box",
              borderBottom: "1px solid rgba(224, 224, 224, 1)",
              paddingTop: 25
            }}
          >
            {columns.map(({ dataKey, ...other }, index) => {
              return (
                <Column
                  key={dataKey}
                  headerRenderer={headerProps =>
                    headerRenderer(order, setOrder, orderBy,setOrderBy, headerHeight, columns,{
                      ...headerProps,
                      columnIndex: index
                    })
                  }
                  // className={classes.flexContainer}
                  cellRenderer={(a)=>cellRenderer(columns, rowHeight, a)}
                  dataKey={dataKey}
                  {...other}
                />
              );
            })}
          </Table>
        )}
      </AutoSizer>
    );
};
