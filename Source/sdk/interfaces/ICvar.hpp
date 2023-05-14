#pragma once

#include "IAppSystem.hpp"
#include "IConVar.hpp"
#include "..\misc\Convar.hpp"
#include <fstream>
#include <string>

class ConCommandBase;
class ConCommand;
class ConVar;

class IConsoleDisplayFunc
{
public:
    virtual void ColorPrint(const uint8_t* clr, const char *pMessage) = 0;
    virtual void Print(const char *pMessage) = 0;
    virtual void DPrint(const char *pMessage) = 0;

    virtual void GetConsoleText(char* pchText, size_t bufSize) const = 0;
};

class ICvarQuery : public IAppSystem
{
public:
    // Can these two convars be aliased?
    virtual bool AreConVarsLinkable(const ConVar * child, const ConVar * parent) = 0;
};

class ICvar : public IAppSystem
{
public:
    // Allocate a unique DLL identifier
    virtual int						    AllocateDLLIdentifier() = 0; 

    // Register, unregister commands
    virtual void                        RegisterConCommand(ConCommandBase *pCommandBase, bool cmdline = true) = 0;
    virtual void                        UnregisterConCommand(ConCommandBase *pCommandBase) = 0;
    virtual void                        UnregisterConCommands(int id) = 0;

    // If there is a +<varname> <value> on the command line, this returns the value.
    // Otherwise, it returns NULL.
    virtual const char*                 GetCommandLineValue(const char* pVariableName) = 0;
    //virtual bool			            HasCommandLineValue(const char* pVariableName) = 0;

    // Try to find the cvar pointer by name
    virtual ConCommandBase*             FindCommandBase(const char* name) = 0;
    virtual const ConCommandBase*       FindCommandBase(const char* name) const = 0;
    virtual ConVar*                     FindVar(const char* var_name) = 0;
    virtual const ConVar*               FindVar(const char* var_name) const = 0;
    virtual ConCommand*                 FindCommand(const char* name) = 0;
    virtual const ConCommand*           FindCommand(const char* name) const = 0;

    // Install a global change callback (to be called when any convar changes)
    virtual void                        InstallGlobalChangeCallback(FnChangeCallback_t callback) = 0;
    virtual void                        RemoveGlobalChangeCallback(FnChangeCallback_t callback) = 0;
    virtual void                        CallGlobalChangeCallbacks(ConVar* var, const char* pOldString, float flOldValue) = 0;

    // Install a console printer
    virtual void                        InstallConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
    virtual void                        RemoveConsoleDisplayFunc(IConsoleDisplayFunc* pDisplayFunc) = 0;
    virtual void                        ConsoleColorPrintf(const Color& clr, const char* pFormat, ...) const = 0;
    virtual void                        ConsolePrintf(const char* pFormat, ...) const = 0;
    virtual void                        ConsoleDPrintf(const char* pFormat, ...) const = 0;

    // Reverts cvars which contain a specific flag
    virtual void                        RevertFlaggedConVars(int nFlag) = 0;

    // Method allowing the engine ICvarQuery interface to take over
    // A little hacky, owing to the fact the engine is loaded
    // well after ICVar, so we can't use the standard connect pattern
    virtual void			            InstallCVarQuery(ICvarQuery* pQuery) = 0;
    virtual void			            SetMaxSplitScreenSlots(int nSlots) = 0;
    virtual int				            GetMaxSplitScreenSlots() const = 0;

    virtual void			            AddSplitScreenConVars() = 0;
    virtual void			            RemoveSplitScreenConVars(int id) = 0;

    virtual int				            GetConsoleDisplayFuncCount() const = 0;
    virtual void			            GetConsoleText(int nDisplayFuncIndex, char* pchText, size_t bufSize) const = 0;

    // Utilities for convars accessed by the material system thread
    virtual bool			            IsMaterialThreadSetAllowed() const = 0;
    virtual void			            QueueMaterialThreadSetValue(ConVar* pConVar, const char* pValue) = 0;
    virtual void			            QueueMaterialThreadSetValue(ConVar* pConVar, int nValue) = 0;
    virtual void			            QueueMaterialThreadSetValue(ConVar* pConVar, float flValue) = 0;
    virtual bool			            HasQueuedMaterialThreadConVarSets() const = 0;
    virtual int				            ProcessQueuedMaterialThreadConVarSets() = 0;

    virtual void			            QueueMaterialThreadSetConCommand(ConCommand* pConCommand) = 0;
    virtual bool			            HasQueuedMaterialThreadConCommandSets() const = 0;
    virtual int				            ProcessQueuedMaterialThreadConCommandSets() = 0;
};
