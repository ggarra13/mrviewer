require 'test_helper'

class LookModTransformsControllerTest < ActionController::TestCase
  setup do
    @look_mod_transform = look_mod_transforms(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:look_mod_transforms)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create look_mod_transform" do
    assert_difference('LookModTransform.count') do
      post :create, look_mod_transform: {  }
    end

    assert_redirected_to look_mod_transform_path(assigns(:look_mod_transform))
  end

  test "should show look_mod_transform" do
    get :show, id: @look_mod_transform
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @look_mod_transform
    assert_response :success
  end

  test "should update look_mod_transform" do
    put :update, id: @look_mod_transform, look_mod_transform: {  }
    assert_redirected_to look_mod_transform_path(assigns(:look_mod_transform))
  end

  test "should destroy look_mod_transform" do
    assert_difference('LookModTransform.count', -1) do
      delete :destroy, id: @look_mod_transform
    end

    assert_redirected_to look_mod_transforms_path
  end
end
