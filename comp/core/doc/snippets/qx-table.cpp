//! [0]
Qx::Table<int> table({
	{0, 1, 2},
	{3, 4, 5},
	{6, 7, 8}
});

Qx::DsvTable<int> sectionTable({
	{1, 2},
	{4, 5},
	{7, 8}
});

qDebug() << sectionTable == table.section(0, 1, 3, 2);
// true
//! [0]