//! [0]
Qx::GroupedProgressManager gpm;

Qx::ProgressGroup* fileGroup = gpm.addGroup("File Copies");
fileGroup->setMaximum(100);
fileGroup->setValue(50); // 50% completion of "File Copies"
fileGroup->setWeight(3);

Qx::ProgressGroup* coolGroup = gpm.addGroup("Cool Stuff");
coolGroup->setMaximum(100);
coolGroup->setValue(0); // 0% completion of "Cool Stuff"
coolGroup->setWeight(7);

quint64 overallProgress = gpm.value(); // overallProgress = 15
//! [0]