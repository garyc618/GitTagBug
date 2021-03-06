import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.util.Random;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.filechooser.FileNameExtensionFilter;

import vtk.*;

public class Focus extends JFrame {

  private vtkRenderWindowPanel mainPanel = new vtkRenderWindowPanel();
  private vtkRenderWindowPanel focusPanel = new vtkRenderWindowPanel();
  private vtkDataRepresentation mainRep = new vtkDataRepresentation();
  private vtkDataRepresentation focusRep = new vtkDataRepresentation();
  
  private vtkDelimitedTextReader reader = new vtkDelimitedTextReader();
  private vtkTableToGraph tableToGraph = new vtkTableToGraph();
  private vtkBoostBreadthFirstSearch mainBFS = new vtkBoostBreadthFirstSearch();
  private vtkBoostBreadthFirstSearch bfs = new vtkBoostBreadthFirstSearch();
  private vtkExtractSelectedGraph extract = new vtkExtractSelectedGraph();
  private vtkGraphLayout layout = new vtkGraphLayout();

  private vtkGraphLayoutView mainView = new vtkGraphLayoutView();
  private vtkGraphLayoutView focusView = new vtkGraphLayoutView();

  private vtkSelectionLink link = new vtkSelectionLink();

  private vtkDoubleArray thresh = new vtkDoubleArray();

  public Focus() {
    // Use a simple ViewChangedObserver to render all views
    // when the selection in one view changes.
    ViewChangedObserver obs = new ViewChangedObserver();
    this.link.AddObserver("SelectionChangedEvent", obs, "SelectionChanged");
    
    this.reader.SetFileName("../../../../VTKData/Data/Infovis/classes.csv");
    this.tableToGraph.SetInputConnection(this.reader.GetOutputPort());
    this.tableToGraph.AddLinkEdge("Field 0", "Field 1");
    vtkSimple2DLayoutStrategy strategy = new vtkSimple2DLayoutStrategy();
    this.layout.SetLayoutStrategy(strategy);
    this.layout.SetInputConnection(this.tableToGraph.GetOutputPort());
    this.mainBFS.SetInputConnection(this.layout.GetOutputPort());
    this.bfs.SetInputConnection(this.mainBFS.GetOutputPort());
    this.extract.SetInputConnection(0, this.bfs.GetOutputPort());
    vtkSelection select = new vtkSelection();
    select.SetContentType(7); // 7 == vtkSelection.THRESHOLDS
    select.SetFieldType(3);   // 3 == vtkSelection.VERTEX
    this.thresh.SetName("BFS");
    this.thresh.InsertNextValue(0);
    this.thresh.InsertNextValue(1);
    select.SetSelectionList(this.thresh);
    this.extract.SetInput(1, select);

    this.mainRep.SetInputConnection(this.bfs.GetOutputPort());
    this.focusRep.SetInputConnection(this.extract.GetOutputPort());

    this.mainView.AddRepresentation(this.mainRep);
    this.mainView.SetVertexLabelArrayName("label");
    this.mainView.VertexLabelVisibilityOn();
    this.mainView.SetLayoutStrategyToPassThrough();
    this.mainView.SetVertexColorArrayName("BFS");
    this.mainView.ColorVerticesOn();
    this.focusView.AddRepresentation(this.focusRep);
    this.focusView.SetVertexLabelArrayName("label");
    this.focusView.VertexLabelVisibilityOn();
    this.focusView.SetVertexColorArrayName("BFS");
    this.focusView.ColorVerticesOn();

    vtkViewTheme t = new vtkViewTheme();
    vtkViewTheme theme = t.CreateMellowTheme();
    theme.SetPointHueRange(0, 0);
    theme.SetPointSaturationRange(1, 0);
    theme.SetPointValueRange(1, 0);
    this.mainView.ApplyViewTheme(theme);
    this.focusView.ApplyViewTheme(theme);

    this.mainView.SetupRenderWindow(this.mainPanel.GetRenderWindow());
    this.focusView.SetupRenderWindow(this.focusPanel.GetRenderWindow());

    // Views should produce pedigree id selections.
    // 2 == vtkSelection.PEDIGREEIDS
    this.mainView.SetSelectionType(2);
    this.focusView.SetSelectionType(2);

    this.mainPanel.setSize(500, 500);
    this.focusPanel.setSize(500, 500);

    //this.mainView.SetLayoutStrategyToSimple2D();
    //this.focusView.SetLayoutStrategyToSimple2D();

    this.mainRep.SetSelectionLink(this.link);
    this.focusRep.SetSelectionLink(this.link);

    // Arrange the views in a grid.
    GridLayout layout = new GridLayout(1, 2);
    layout.setHgap(5);
    layout.setVgap(5);
    JPanel panel = new JPanel();
    panel.setLayout(layout);
    panel.add(mainPanel);
    panel.add(focusPanel);

    // Create menu
    JMenuBar menuBar;
    JMenu menu;
    JMenuItem menuItem;

    // Create the menu bar.
    menuBar = new JMenuBar();

    // Build the first menu.
    menu = new JMenu("File");
    menu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(menu);

    // A JMenuItem
    menuItem = new JMenuItem("Open Edge List...", KeyEvent.VK_X);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O,
        ActionEvent.CTRL_MASK));
    menuItem.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        // Create a file chooser
        final JFileChooser fc = new JFileChooser();
        // In response to a button click:
        int returnVal = fc.showOpenDialog(null);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
          File f = fc.getSelectedFile();
          reader.SetFileName(f.getAbsolutePath());
          ViewChangedObserver obs = new ViewChangedObserver();
          obs.SelectionChanged();
        }
      }});
    menu.add(menuItem);

    JToolBar toolbar = new JToolBar();

    // A JButton
    JButton increase = new JButton("Increase Focus");
    increase.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        thresh.SetValue(1, thresh.GetValue(1) + 1);
	extract.Modified();
	update();
      }});
    toolbar.add(increase);

    // A JButton
    JButton decrease = new JButton("Decrease Focus");
    decrease.addActionListener(new ActionListener() {
      @Override
      public void actionPerformed(ActionEvent e) {
        if (thresh.GetValue(1) > 0) {
          thresh.SetValue(1, thresh.GetValue(1) - 1);
	  extract.Modified();
          update();
        }
      }});
    toolbar.add(decrease);

    this.setJMenuBar(menuBar);
    this.add(toolbar, BorderLayout.NORTH);
    this.add(panel);
  }

  void update() {
    this.mainPanel.Render();
    this.focusPanel.Render();
    this.mainView.GetRenderer().ResetCamera();
    this.focusView.GetRenderer().ResetCamera();
    this.mainPanel.Render();
    this.focusPanel.Render();
  }

  private class ViewChangedObserver {
    // When a selection changes, render all views.
    void SelectionChanged() {
      vtkSelection currentSel = link.GetSelection();
      vtkConvertSelection convert = new vtkConvertSelection();
      vtkSelection indexSel =
        convert.ToIndexSelection(currentSel, tableToGraph.GetOutput());
      if (currentSel.GetNumberOfChildren() > 0) {
        vtkIdTypeArray ids = (vtkIdTypeArray)indexSel.GetChild(0).
          GetSelectionList();
        if (ids != null && ids.GetNumberOfTuples() == 1) {
          bfs.SetOriginVertex(ids.GetValue(0));
          bfs.Modified();
          focusPanel.Render();
          focusView.GetRenderer().ResetCamera();
        }
      }
      mainPanel.Render();
      focusPanel.Render();
    }
  }

  // In the static constructor we load in the native code.
  // The libraries must be in your path to work.
  static {
    System.loadLibrary("vtkCommonJava");
    System.loadLibrary("vtkFilteringJava");
    System.loadLibrary("vtkIOJava");
    System.loadLibrary("vtkImagingJava");
    System.loadLibrary("vtkGraphicsJava");
    System.loadLibrary("vtkRenderingJava");
    System.loadLibrary("vtkInfovisJava");
    System.loadLibrary("vtkViewsJava");
  }

  public static void main(String args[]) {
    JPopupMenu.setDefaultLightWeightPopupEnabled(false);

    final Focus app = new Focus();
    javax.swing.SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        app.setTitle("Focus");
        app.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        app.pack();
        app.setVisible(true);
        app.addWindowListener(new WindowAdapter() {
          @Override
          public void windowClosing(WindowEvent e) {
            // Calling vtkGlobalJavaHash.DeleteAll() will clean up
            // VTK references before the Java program exits.
            vtkGlobalJavaHash.DeleteAll();
          }
        });
        app.update();
      }});
  }

}
