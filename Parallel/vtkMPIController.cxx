/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkMPIController.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMPIController.h"

#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"

#include "vtkMPI.h"

#include "vtkSmartPointer.h"
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

int vtkMPIController::Initialized = 0;
char vtkMPIController::ProcessorName[MPI_MAX_PROCESSOR_NAME] = "";
int vtkMPIController::UseSsendForRMI = 0;

// Output window which prints out the process id
// with the error or warning messages
class VTK_PARALLEL_EXPORT vtkMPIOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeRevisionMacro(vtkMPIOutputWindow,vtkOutputWindow);

  void DisplayText(const char* t)
    {
      if (this->Controller && vtkMPIController::Initialized)
        {
        cout << "Process id: " << this->Controller->GetLocalProcessId()
             << " >> ";
        }
      cout << t;
    }

  vtkMPIOutputWindow()
    {
      vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMPIOutputWindow");
      if (ret)
        ret->Delete();
      this->Controller = 0;
    }

  friend class vtkMPIController;

protected:

  vtkMPIController* Controller;
  vtkMPIOutputWindow(const vtkMPIOutputWindow&);
  void operator=(const vtkMPIOutputWindow&);

};

void vtkMPIController::CreateOutputWindow()
{
  vtkMPIOutputWindow* window = new vtkMPIOutputWindow;
  window->Controller = this;
  this->OutputWindow = window;
  vtkOutputWindow::SetInstance(this->OutputWindow);
}

vtkCxxRevisionMacro(vtkMPIOutputWindow, "1.27");

vtkCxxRevisionMacro(vtkMPIController, "1.27");
vtkStandardNewMacro(vtkMPIController);

//----------------------------------------------------------------------------
vtkMPIController::vtkMPIController()
{
  // If MPI was already initialized obtain rank and size.
  if (vtkMPIController::Initialized)
    {
    this->InitializeCommunicator(vtkMPICommunicator::GetWorldCommunicator());
    // Copy vtkMPIController::WorldRMICommunicataor which is created when
    // MPI is initialized
    vtkMPICommunicator* comm = vtkMPICommunicator::New();
    comm->CopyFrom(vtkMPIController::WorldRMICommunicator);
    this->RMICommunicator = comm;
    }

  this->OutputWindow = 0;
}

//----------------------------------------------------------------------------
vtkMPIController::~vtkMPIController()
{
  this->SetCommunicator(0);
  if (this->RMICommunicator)
    {
    this->RMICommunicator->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkMPIController::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Initialized: " << ( vtkMPIController::Initialized ? "(yes)" : "(no)" ) << endl;
}

vtkMPICommunicator* vtkMPIController::WorldRMICommunicator=0;

//----------------------------------------------------------------------------
void vtkMPIController::TriggerRMIInternal(int remoteProcessId, 
    void* arg, int argLength, int rmiTag, bool propagate)
{
  vtkMPICommunicator* mpiComm = vtkMPICommunicator::SafeDownCast(
    this->RMICommunicator);
  int use_ssend = mpiComm->GetUseSsend(); 
  if (vtkMPIController::UseSsendForRMI == 1 && use_ssend == 0)
    {
    mpiComm->SetUseSsend(1);
    }

  this->Superclass::TriggerRMIInternal(remoteProcessId,
    arg, argLength, rmiTag, propagate);

  if (vtkMPIController::UseSsendForRMI == 1 && use_ssend == 0)
    {
    mpiComm->SetUseSsend(0);
    }
}

//----------------------------------------------------------------------------
void vtkMPIController::Initialize(int* argc, char*** argv, 
                                  int initializedExternally)
{
  if (vtkMPIController::Initialized)
    {
    vtkWarningMacro("Already initialized.");
    return;
    }
  
  // Can be done once in the program.
  vtkMPIController::Initialized = 1;
  if (initializedExternally == 0)
    {
    MPI_Init(argc, argv);
    }
  this->InitializeCommunicator(vtkMPICommunicator::GetWorldCommunicator());

  int tmp;
  MPI_Get_processor_name(ProcessorName, &tmp);
  // Make a copy of MPI_COMM_WORLD creating a new context.
  // This is used in the creating of the communicators after
  // Initialize() has been called. It has to be done here
  // because for this to work, all processes have to call
  // MPI_Comm_dup and this is the only method which is
  // guaranteed to be called by all processes.
  vtkMPIController::WorldRMICommunicator = vtkMPICommunicator::New();
  vtkMPIController::WorldRMICommunicator->Duplicate((vtkMPICommunicator*)this->Communicator);
  this->RMICommunicator = vtkMPIController::WorldRMICommunicator;
  // Since we use Delete to get rid of the reference, we should use NULL to register.
  this->RMICommunicator->Register(NULL);

  this->Modified();
}

const char* vtkMPIController::GetProcessorName()
{
  return ProcessorName;
}

// Good-bye world
// There should be no MPI calls after this.
// (Except maybe MPI_XXX_free()) unless finalized externally.
void vtkMPIController::Finalize(int finalizedExternally)
{
  if (vtkMPIController::Initialized)
    { 
    vtkMPIController::WorldRMICommunicator->Delete();
    vtkMPIController::WorldRMICommunicator = 0;
    vtkMPICommunicator::WorldCommunicator->Delete();
    vtkMPICommunicator::WorldCommunicator = 0;
    this->SetCommunicator(0);
    if (this->RMICommunicator)
      {
      this->RMICommunicator->Delete();
      this->RMICommunicator = 0;
      }
    if (finalizedExternally == 0)
      {
      MPI_Finalize();
      }
    vtkMPIController::Initialized = 0;
    this->Modified();
    }  
  
}

// Called by SetCommunicator and constructor. It frees but does 
// not set RMIHandle (which should not be set by using MPI_Comm_dup
// during construction).
void vtkMPIController::InitializeCommunicator(vtkMPICommunicator* comm)
{
  if (this->Communicator != comm) 
    { 
    if (this->Communicator != 0) 
      { 
      this->Communicator->UnRegister(this); 
      }
    this->Communicator = comm; 
    if (this->Communicator != 0) 
      { 
      this->Communicator->Register(this); 
      } 

    this->Modified(); 
    }  


}

// Delete the previous RMI communicator and creates a new one
// by duplicating the user communicator.
void vtkMPIController::InitializeRMICommunicator()
{
  if ( this->RMICommunicator )
    {
    this->RMICommunicator->Delete();
    this->RMICommunicator = 0;
    }
  if (this->Communicator)
    {
    this->RMICommunicator = vtkMPICommunicator::New();
    ((vtkMPICommunicator*)this->RMICommunicator)->Duplicate((vtkMPICommunicator*)this->Communicator);
    }
}

void vtkMPIController::SetCommunicator(vtkMPICommunicator* comm)
{
  this->InitializeCommunicator(comm);
  this->InitializeRMICommunicator();
}

//----------------------------------------------------------------------------
// Execute the method set as the SingleMethod.
void vtkMPIController::SingleMethodExecute()
{
  if(!vtkMPIController::Initialized)
    {
    vtkWarningMacro("MPI has to be initialized first.");
    return;
    }

  if (this->GetLocalProcessId() < this->GetNumberOfProcesses())
    {
    if (this->SingleMethod)
      {
      vtkMultiProcessController::SetGlobalController(this);
      (this->SingleMethod)(this, this->SingleData);
      }
    else
      {
      vtkWarningMacro("SingleMethod not set.");
      }
    }
}

//----------------------------------------------------------------------------
// Execute the methods set as the MultipleMethods.
void vtkMPIController::MultipleMethodExecute()
{
  if(!vtkMPIController::Initialized)
    {
    vtkWarningMacro("MPI has to be initialized first.");
    return;
    }

  int i = this->GetLocalProcessId();
  
  if (i < this->GetNumberOfProcesses())
    {
    if (this->MultipleMethod[i])
      {
      vtkMultiProcessController::SetGlobalController(this);
      (this->MultipleMethod[i])(this, this->MultipleData[i]);
      }
    else
      {
      vtkWarningMacro("MultipleMethod " << i << " not set.");
      }
    }
}

char* vtkMPIController::ErrorString(int err)
{
  char* buffer = new char[MPI_MAX_ERROR_STRING];
  int resLen;
  MPI_Error_string(err, buffer, &resLen);
  return buffer;
}

//-----------------------------------------------------------------------------
vtkMPIController *vtkMPIController::CreateSubController(vtkProcessGroup *group)
{
  VTK_CREATE(vtkMPICommunicator, subcomm);

  if (!subcomm->Initialize(group)) return NULL;

  // MPI is kind of funny in that in order to create a communicator from a
  // subgroup of another communicator, it is a collective operation involving
  // all of the processes in the original communicator, not just those belonging
  // to the group.  In any process not part of the group, the communicator is
  // created with MPI_COMM_NULL.  Check for that and return NULL ourselves,
  // which is not really an error condition.
  if (*(subcomm->GetMPIComm()->Handle) == MPI_COMM_NULL) return NULL;

  vtkMPIController *controller = vtkMPIController::New();
  controller->SetCommunicator(subcomm);
  return controller;
}
